// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include <boost/program_options.hpp>
#include <string>
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <iostream>
#include <fstream>
#include "MCHBase/Digit.h"
#include <fmt/format.h>
#include "CommonDataFormat/InteractionRecord.h"
#include "MCHRawEncoder/DigitEncoder.h"
#include "MCHRawCommon/DataFormats.h"
#include "MCHRawElecMap/Mapper.h"
#include "Digit2RawConverter.h"
#include "Headers/RAWDataHeader.h"
#include <limits>

namespace po = boost::program_options;
using namespace o2::mch::raw;

int main(int argc, char* argv[])
{
  po::options_description generic("options");
  bool userLogic{false};
  bool dummyElecMap{false};
  std::string input;
  std::string output;
  int maxdigits{std::numeric_limits<int>::max()};
  po::variables_map vm;

  // clang-format off
  generic.add_options()
      ("help,h", "produce help message")
      ("userLogic,u",po::bool_switch(&userLogic),"user logic format")
      ("dummyElecMap,d",po::bool_switch(&dummyElecMap),"use a dummy electronic mapping (for testing only)")
      ("outfile,o",po::value<std::string>(&output)->required(),"output filename")
      ("infile,i",po::value<std::string>(&input)->required(),"input file name")
      ("nmax,n",po::value<int>(&maxdigits),"max number of digits")
      ;
  // clang-format on

  po::options_description cmdline;
  cmdline.add(generic);

  po::store(po::command_line_parser(argc, argv).options(cmdline).run(), vm);

  if (vm.count("help")) {
    std::cout << generic << "\n";
    return 2;
  }

  try {
    po::notify(vm);
  } catch (boost::program_options::error& e) {
    std::cout << "Error: " << e.what() << "\n";
    exit(1);
  }

  po::notify(vm);

  TFile fi(input.c_str());
  TTree* o2sim;
  fi.GetObject("o2sim", o2sim);
  TBranch* digitBranch = o2sim->GetBranch("MCHDigit");
  std::vector<o2::mch::Digit>* digits;
  digitBranch->SetAddress(&digits);

  digitBranch->Print();
  digitBranch->GetEntry(0);

  // builds a list of digit per interaction record
  std::map<o2::InteractionTimeRecord, std::vector<o2::mch::Digit>> digitsPerIR;
  int n{0};
  for (auto d : (*digits)) {
    o2::InteractionTimeRecord ir(d.getTimeStamp());
    digitsPerIR[ir].push_back(d);
    n++;
    if (n > maxdigits) {
      break;
    }
  }

  // rescale the timetamp of each digit to be relative to BC
  // FIXME
  for (auto p : digitsPerIR) {
    std::cout << fmt::format("ORBIT {:6d} BC {:4d} nofDigits {:5d}\n",
                             p.first.orbit, p.first.bc, p.second.size());
    for (auto& d : p.second) {
      d = o2::mch::Digit(0, d.getDetID(), d.getPadID(), d.getADC());
    }
  }

  std::cout << n << "\n";
  //auto det2elec = createDet2ElecMapper<ElectronicMapperGenerated>(deIdsOfCH5L /*deIdsForAllMCH*/);
  //auto solar2cru = createSolar2CruLinkMapper<ElectronicGenerated>();

  auto det2elec = createDet2ElecMapper<ElectronicMapperDummy>(deIdsForAllMCH);
  auto solar2cru = createSolar2CruLinkMapper<ElectronicMapperDummy>();

  DigitEncoder encoder = createDigitEncoder(userLogic, det2elec);

  std::ofstream out(output);

  digit2raw<o2::header::RAWDataHeaderV4>(digitsPerIR, encoder, solar2cru, out);

  return 0;
}
