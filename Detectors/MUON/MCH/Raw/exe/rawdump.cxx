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

namespace po = boost::program_options;

extern std::ostream& operator<<(std::ostream&, const o2::header::RAWDataHeaderV4&);

int rawdump(std::string input, unsigned int maxNofRDHs, bool showRDHs)
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

  memset(&buffer[0], 0, buffer.size());
  auto hp = [](uint8_t cruId, uint8_t linkId, uint8_t chip,
               uint8_t channel, o2::mch::raw::SampaCluster sc) {
    std::cout << fmt::format("CHIP {:2d} CH {:2d} ", chip, channel);
    std::cout << sc << "\n";
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

  auto decode = o2::mch::raw::createBareDecoder<o2::header::RAWDataHeaderV4>(rh, hp, false);

  std::vector<std::chrono::microseconds> timers;

  while (pos + sizeToRead <= len && nrdhs < maxNofRDHs) {
    in.seekg(pos);
    in.read(ptr, sizeToRead);
    pos += sizeToRead;
    auto start = std::chrono::high_resolution_clock::now();
    decode(buffer);
    // std::cout << "bufer.size()=" << buffer.size() << "\n";
    // o2::mch::raw::dumpBuffer(buffer);
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
  bool showRDHs;

  // clang-format off
  generic.add_options()
      ("help", "produce help message")
      ("input-file,i", po::value<std::string>(&inputFile), "input file name")
      ("nrdhs,n", po::value<unsigned int>(&nrdhs), "number of RDHs to go through")
      ("showRDHs,s",po::bool_switch(&showRDHs),"show RDHs")
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

  return rawdump(inputFile, nrdhs, showRDHs);
}
