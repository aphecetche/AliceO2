// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

///
/// @author  Laurent Aphecetche

#include "boost/program_options.hpp"
#include <iostream>
#include <fstream>
#include <gsl/span>
#include <fmt/format.h>
#include "MCHRawDecoder/Decoder.h"
#include "Headers/RAWDataHeader.h"
#include <chrono>
#include "DumpBuffer.h"
#include "MCHRawCommon/DataFormats.h"

namespace po = boost::program_options;

extern std::ostream& operator<<(std::ostream&, const o2::header::RAWDataHeaderV4&);

using namespace o2::mch::raw;
using RDHv4 = o2::header::RAWDataHeaderV4;

class DumpOptions
{
 public:
  DumpOptions(uint16_t solarOffset, unsigned int maxNofRDHs, bool showRDHs)
    : mSolarOffset{solarOffset},
      mMaxNofRDHs{maxNofRDHs == 0 ? std::numeric_limits<unsigned int>::max() : maxNofRDHs},
      mShowRDHs{showRDHs} {}

  unsigned int maxNofRDHs() const
  {
    return mMaxNofRDHs;
  }

  uint16_t solarOffset() const
  {
    return mSolarOffset;
  }

  bool showRDHs() const
  {
    return mShowRDHs;
  }

 private:
  uint16_t mSolarOffset;
  unsigned int mMaxNofRDHs;
  bool mShowRDHs;
};

struct Stat {
  double mean{0};
  double rms{0};
  double q{0};
  int n{0};
  void incr(int v)
  {
    n++;
    auto newMean = mean + (v - mean) / n;
    q += (v - newMean) * (v - mean);
    rms = sqrt(q / n);
    mean = newMean;
  }
};

std::ostream& operator<<(std::ostream& os, const Stat& s)
{
  os << fmt::format("MEAN {:7.3f} RMS {:7.3f} NSAMPLES {:5d} ", s.mean, s.rms, s.n);
  return os;
}
template <typename FORMAT, typename CHARGESUM, typename RDH>
int rawdump(std::string input, DumpOptions opt)
{
  std::ifstream in(input.c_str(), std::ios::binary);
  if (!in.good()) {
    std::cout << "could not open file " << input << "\n";
    return 1;
  }
  constexpr size_t pageSize = 8192;

  std::array<uint8_t, pageSize> buffer;
  gsl::span<uint8_t> sbuffer(buffer);

  size_t ndigits{0};

  std::map<std::string, int> uniqueDS;
  std::map<std::string, int> uniqueChannel;
  std::map<std::string, Stat> statChannel;

  memset(&buffer[0], 0, buffer.size());
  auto channelHandler = [&ndigits, &uniqueDS, &uniqueChannel, &statChannel](DsElecId dsId,
                                                                            uint8_t channel, o2::mch::raw::SampaCluster sc) {
    auto s = asString(dsId);
    uniqueDS[s]++;
    auto ch = fmt::format("{}-CH-{}", s, channel);
    uniqueChannel[ch]++;
    auto& stat = statChannel[ch];
    for (auto d = 0; d < sc.nofSamples(); d++) {
      stat.incr(sc.samples[d]);
    }
    ++ndigits;
  };

  size_t nrdhs{0};
  auto rdhHandler = [&](const RDH& rdh) {
    nrdhs++;
    if (opt.showRDHs()) {
      std::cout << nrdhs << "--" << rdh << "\n";
    }
    auto r = rdh;
    r.feeId = rdhLinkId(r) + opt.solarOffset();
    return r;
  };

  o2::mch::raw::Decoder decode = o2::mch::raw::createDecoder<FORMAT, CHARGESUM, RDH>(rdhHandler, channelHandler);

  std::vector<std::chrono::microseconds> timers;

  size_t npages{0};

  while (npages < opt.maxNofRDHs() && in.read(reinterpret_cast<char*>(&buffer[0]), pageSize)) {
    npages++;
    int n = decode(sbuffer);
  }

  std::cout << ndigits << " digits seen - " << nrdhs << " RDHs seen - " << npages << " npages read\n";
  std::cout << "#unique DS=" << uniqueDS.size() << " #unique Channel=" << uniqueChannel.size() << "\n";
  for (auto s : uniqueDS) {
    std::cout << s.first << " " << s.second << "\n";
  }
  std::cout << "--------\n";
  for (auto s : uniqueChannel) {
    std::cout << fmt::format("{:18s}", s.first) << statChannel.at(s.first) << fmt::format("NCLU {:5d}\n", s.second);
  }
  return 0;
}

int main(int argc, char* argv[])
{
  std::string prefix;
  std::vector<int> detElemIds;
  std::string inputFile;
  po::variables_map vm;
  po::options_description generic("Generic options");
  unsigned int nrdhs{0};
  uint16_t solarOffset{0};
  bool showRDHs{false};
  bool userLogic{false}; // default format is bareformat...
  bool chargeSum{false}; //... in sample mode

  // clang-format off
  generic.add_options()
      ("help,h", "produce help message")
      ("input-file,i", po::value<std::string>(&inputFile), "input file name")
      ("nrdhs,n", po::value<unsigned int>(&nrdhs), "number of RDHs to go through")
      ("showRDHs,s",po::bool_switch(&showRDHs),"show RDHs")
      ("userLogic,u",po::bool_switch(&userLogic),"user logic format")
      ("chargeSum,c",po::bool_switch(&chargeSum),"charge sum format")
      ("solarOffset,o",po::value<uint16_t>(&solarOffset),"offset to add to linkID to get a solarID")
      ;
  // clang-format on

  po::options_description cmdline;
  cmdline.add(generic);

  po::store(po::command_line_parser(argc, argv).options(cmdline).run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << generic << "\n";
    return 2;
  }

  DumpOptions opt(solarOffset, nrdhs, showRDHs);

  if (userLogic) {
    if (chargeSum) {
      return rawdump<UserLogicFormat, ChargeSumMode, RDHv4>(inputFile, opt);
    } else {
      return rawdump<UserLogicFormat, SampleMode, RDHv4>(inputFile, opt);
    }
  } else {
    if (chargeSum) {
      return rawdump<BareFormat, ChargeSumMode, RDHv4>(inputFile, opt);
    } else {
      return rawdump<BareFormat, SampleMode, RDHv4>(inputFile, opt);
    }
  }
}
