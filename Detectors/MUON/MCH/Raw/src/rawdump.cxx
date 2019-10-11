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
#include "MCHRaw/Decoder.h"

namespace po = boost::program_options;

int rawdump(std::string input)
{
  std::ifstream in(input.c_str(), std::ios::binary);
  if (!in.good()) {
    std::cout << "could not open file " << input << "\n";
    return 1;
  }
  constexpr int sizeToRead = 8192;

  std::array<uint32_t, sizeToRead> buffer;
  char* ptr = reinterpret_cast<char*>(&buffer[0]);

  auto hp = [](o2::mch::raw::SampaHit sh) {
    std::cout << fmt::format("CHIP {:2d} CH {:2d} TS {:5d} Q {:5d}\n",
                             sh.chip, sh.channel, sh.timestamp, sh.chargeSum);
  };

  size_t nrdhs{0};
  auto rh = [&nrdhs](const o2::mch::raw::RAWDataHeader& rdh) {
    nrdhs++;
    std::cout << nrdhs << "--" << rdh << "\n";
    return true;
  };

  size_t pos{0};
  in.seekg(0, in.end);
  auto len = in.tellg();
  in.seekg(0, in.beg);

  auto decode = o2::mch::raw::createBareDecoder(rh, hp, false);

  while (pos + sizeToRead < len) {
    in.seekg(pos);
    in.read(ptr, sizeToRead);
    pos += sizeToRead;
    // o2::mch::raw::dumpBuffer(buffer);
    decode(buffer);
  }

  // o2::mch::raw::dumpBuffer(buffer);

  return 0;
}

int main(int argc, char* argv[])
{
  std::string prefix;
  std::vector<int> detElemIds;
  std::string inputFile;
  po::variables_map vm;
  po::options_description generic("Generic options");

  generic.add_options()("help", "produce help message")("input-file,i", po::value<std::string>(&inputFile), "input file name");

  po::options_description cmdline;
  cmdline.add(generic);

  po::store(po::command_line_parser(argc, argv).options(cmdline).run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << generic << "\n";
    return 2;
  }

  return rawdump(inputFile);
}
