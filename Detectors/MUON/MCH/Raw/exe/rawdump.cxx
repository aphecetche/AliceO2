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

namespace po = boost::program_options;

extern std::ostream& operator<<(std::ostream&, const o2::header::RAWDataHeaderV4&);

int rawdump(std::string input, unsigned int maxNofRDHs, bool showRDHs, bool userLogic, bool chargeSum)
{
  if (maxNofRDHs == 0) {
    maxNofRDHs = std::numeric_limits<unsigned int>::max();
  }
  std::ifstream in(input.c_str(), std::ios::binary);
  if (!in.good()) {
    std::cout << "could not open file " << input << "\n";
    return 1;
  }
  constexpr int sizeToRead = 8192;

  std::array<uint8_t, sizeToRead> buffer;
  char* ptr = reinterpret_cast<char*>(&buffer[0]);

  size_t ndigits{0};

  memset(&buffer[0], 0, buffer.size());
  auto hp = [&ndigits](uint8_t cruId, uint8_t linkId, uint8_t chip,
                       uint8_t channel, o2::mch::raw::SampaCluster sc) {
    std::cout << fmt::format("CRU {:2d} LINK {:4d} CHIP {:2d} CH {:2d} ", cruId, linkId, chip, channel);
    std::cout << sc << "\n";
    ++ndigits;
  };

  size_t nrdhs{0};
  auto rh = [&](const o2::header::RAWDataHeaderV4& rdh) {
    nrdhs++;
    if (showRDHs) {
      std::cout << nrdhs << "--" << rdh << "\n";
    }
    return true;
  };

  size_t pos{0};
  in.seekg(0, in.end);
  auto len = in.tellg();
  in.seekg(0, in.beg);

  o2::mch::raw::Decoder decode;

  if (userLogic) {
    decode = o2::mch::raw::createUserLogicDecoder<o2::header::RAWDataHeaderV4>(rh, hp, chargeSum);
  } else {
    decode = o2::mch::raw::createBareDecoder<o2::header::RAWDataHeaderV4>(rh, hp, chargeSum);
  }

  std::vector<std::chrono::microseconds> timers;

  while (pos + sizeToRead <= len && nrdhs < maxNofRDHs) {
    in.seekg(pos);
    in.read(ptr, sizeToRead);
    pos += sizeToRead;
    auto start = std::chrono::high_resolution_clock::now();
    decode(buffer);
    // std::cout << "bufer.size()=" << buffer.size() << "\n";
    // o2::mch::raw::impl::dumpBuffer(gsl::span<uint8_t>(buffer));
    auto duration = (std::chrono::high_resolution_clock::now() - start);
    timers.push_back(std::chrono::duration_cast<std::chrono::microseconds>(duration));
  }

  std::ofstream out("rawdump.timing.txt");
  int p{0};
  for (auto d : timers) {
    p++;
    out << p << " " << d.count() << "\n";
  }
  out.close();

  std::cout << ndigits << " digits seen\n";
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
      ("help:h", "produce help message")
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

  return rawdump(inputFile, nrdhs, showRDHs, userLogic, chargeSum);
}
