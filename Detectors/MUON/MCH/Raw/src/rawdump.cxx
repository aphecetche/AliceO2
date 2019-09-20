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
#include "MCHMappingInterface/Segmentation.h"
#include <iostream>

using namespace o2::mch::mapping;

namespace po = boost::program_options;

void rawdump(int detElemId)
{
  Segmentation seg{detElemId};
  std::cout << "DE " << detElemId << seg.nofPads() << "\n";
}

int main(int argc, char* argv[])
{
  std::string prefix;
  std::vector<int> detElemIds;
  po::variables_map vm;
  po::options_description generic("Generic options");

  generic.add_options()("help", "produce help message")("de", po::value<std::vector<int>>(&detElemIds),
                                                        "which detection element to consider");

  po::options_description cmdline;
  cmdline.add(generic);

  po::store(po::command_line_parser(argc, argv).options(cmdline).run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << generic << "\n";
    return 2;
  }

  if (detElemIds.empty()) {
    std::cout << "Must give at least one detection element id to work with\n";
    std::cout << generic << "\n";
    return 3;
  }

  for (auto& detElemId : detElemIds) {
    rawdump(detElemId);
  }

  return 0;
}
