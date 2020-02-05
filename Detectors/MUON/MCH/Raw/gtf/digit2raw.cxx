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
#include "MCHBase/Digit.h"
#include <fmt/format.h>
#include "CommonDataFormat/InteractionRecord.h"
#include "MCHRawEncoder/Encoder.h"
#include "DigitEncoder.h"
#include "MCHRawCommon/DataFormats.h"
#include "MCHRawElecMap/Mapper.h"

namespace po = boost::program_options;
using namespace o2::mch::raw;

int main(int argc, char* argv[])
{
  po::options_description generic("options");
  bool userLogic{false};
  bool chargeSum{false};
  bool dummyElecMap{false};
  std::string input;
  std::string output;
  po::variables_map vm;

  // clang-format off
  generic.add_options()
      ("help,h", "produce help message")
      ("userLogic,u",po::bool_switch(&userLogic),"user logic format")
      ("chargesum,c",po::bool_switch(&chargeSum),"charge sum mode")
      ("dummyElecMap,d",po::bool_switch(&dummyElecMap),"use a dummy electronic mapping (for testing only)")
      ("outfile,o",po::value<std::string>(&output)->required(),"output filename")
      ("infile,i",po::value<std::string>(&input)->required(),"input file name")
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
  std::map<o2::InteractionRecord, std::vector<o2::mch::Digit>> digitsPerIR;
  for (auto d : (*digits)) {
    o2::InteractionTimeRecord ir(d.getTimeStamp());
    digitsPerIR[ir].push_back(d);
  }

  // rescale the timetamp of each digit to be relative to BC
  // FIXME
  for (auto p : digitsPerIR) {
    for (auto& d : p.second) {
      d = o2::mch::Digit(0, d.getDetID(), d.getPadID(), d.getADC());
    }
  }

  auto det2elec = createDet2ElecMapper<ElectronicMapperDummy>(deIdsForAllMCH);
  DigitEncoder encoder;

  if (userLogic) {
    if (chargeSum) {
      encoder = createDigitEncoder<UserLogicFormat, ChargeSumMode>(det2elec);
    } else {
      encoder = createDigitEncoder<UserLogicFormat, SampleMode>(det2elec);
    }
  } else {
    if (chargeSum) {
      encoder = createDigitEncoder<BareFormat, ChargeSumMode>(det2elec);
    } else {
      encoder = createDigitEncoder<BareFormat, SampleMode>(det2elec);
    }
  }

  std::vector<uint8_t> buffer;
  for (auto p : digitsPerIR) {
    auto& digits = p.second;
    encoder(digits, buffer, p.first.orbit, p.first.bc);
  }
  return 0;
}
