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

template <typename FORMAT, typename CHARGESUM, typename RDH>
int rawdump(std::string input, unsigned int maxNofRDHs, bool showRDHs)
{
  if (maxNofRDHs == 0) {
    maxNofRDHs = std::numeric_limits<unsigned int>::max();
  }
  std::cout << __PRETTY_FUNCTION__ << " maxNofRDHs=" << maxNofRDHs << "\n";

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

  memset(&buffer[0], 0, buffer.size());
  auto channelHandler = [&ndigits, &uniqueDS, &uniqueChannel](DsElecId dsId,
                                                              uint8_t channel, o2::mch::raw::SampaCluster sc) {
    std::stringstream ss;
    ss << dsId;
    auto s = ss.str();
    uniqueDS[s]++;
    uniqueChannel[fmt::format("{}-CH-{}", s, channel)]++;
    ++ndigits;
  };

  size_t nrdhs{0};
  auto rdhHandler = [&](const RDH& rdh) {
    nrdhs++;
    if (showRDHs) {
      std::cout << nrdhs << "--" << rdh << "\n";
    }
    return rdh;
  };

  o2::mch::raw::Decoder decode = o2::mch::raw::createDecoder<FORMAT, CHARGESUM, RDH>(rdhHandler, channelHandler);

  std::vector<std::chrono::microseconds> timers;

  size_t npages{0};

  while (npages < maxNofRDHs && in.read(reinterpret_cast<char*>(&buffer[0]), pageSize)) {
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
    std::cout << s.first << " " << s.second << "\n";
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
  bool showRDHs{false};
  bool userLogic{false};
  bool chargeSum{false};

  // clang-format off
  generic.add_options()
      ("help,h", "produce help message")
      ("input-file,i", po::value<std::string>(&inputFile), "input file name")
      ("nrdhs,n", po::value<unsigned int>(&nrdhs), "number of RDHs to go through")
      ("showRDHs,s",po::bool_switch(&showRDHs),"show RDHs")
      ("userLogic,u",po::bool_switch(&userLogic),"user logic format")
      ("chargeSum,c",po::bool_switch(&chargeSum),"charge sum format")
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

  if (userLogic) {
    if (chargeSum) {
      return rawdump<UserLogicFormat, ChargeSumMode, RDHv4>(inputFile, nrdhs, showRDHs);
    } else {
      return rawdump<UserLogicFormat, SampleMode, RDHv4>(inputFile, nrdhs, showRDHs);
    }
  } else {
    if (chargeSum) {
      return rawdump<BareFormat, ChargeSumMode, RDHv4>(inputFile, nrdhs, showRDHs);
    } else {
      return rawdump<BareFormat, SampleMode, RDHv4>(inputFile, nrdhs, showRDHs);
    }
  }
}
