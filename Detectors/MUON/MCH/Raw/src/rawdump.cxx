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

void packetHandler(uint8_t chip, uint8_t channel, uint16_t timetamp,
                   uint32_t chargeSum)
{
  std::cout << " chip= " << (int)chip << " ch= " << (int)channel << " ts=" << (int)timetamp << " q=" << (int)chargeSum
            << "\n";
}

bool rdhHandler(const o2::mch::raw::RAWDataHeader& rdh)
{
  std::cout << rdh << "\n";
  return true;
}

int rawdump(std::string input)
{
  std::ifstream in(input.c_str(), std::ios::binary);
  if (!in.good()) {
    std::cout << "could not open file " << input << "\n";
    return 1;
  }
  constexpr int sizeToRead = 8192 * 2;

  std::array<uint32_t, sizeToRead> buffer;

  char* ptr = reinterpret_cast<char*>(&buffer[0]);
  in.read(ptr, sizeToRead);

  // o2::mch::raw::dumpBuffer(buffer);
  // o2::mch::raw::showRDHs(buffer);

  auto decode = o2::mch::raw::createBareDecoder(rdhHandler, packetHandler);
  decode(buffer);

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
