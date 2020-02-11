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
#include "DetectorsRaw/HBFUtils.h"

namespace po = boost::program_options;
using namespace o2::mch::raw;

namespace
{
std::string asString(const o2::InteractionRecord& ir)
{
  return fmt::format("ORB {:6d} BC {:4d}",
                     ir.orbit, ir.bc);
}

std::string asString(const o2::InteractionTimeRecord& ir)
{
  return asString(static_cast<o2::InteractionRecord>(ir));
}
} // namespace

std::ostream& operator<<(std::ostream& os, const o2::mch::Digit& d)
{
  os << fmt::format("DE {:4d} PADUID {:8d} ADC {:6d} TS {:g}",
                    d.getDetID(), d.getPadID(), d.getADC(),
                    d.getTimeStamp());
  return os;
}

int main(int argc, char* argv[])
{
  po::options_description generic("options");
  bool userLogic{false};
  bool dummyElecMap{false};
  std::string input;
  std::string output;
  po::variables_map vm;

  // clang-format off
  generic.add_options()
      ("help,h", "produce help message")
      ("userLogic,u",po::bool_switch(&userLogic),"user logic format")
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

  o2::raw::HBFUtils hbfutils;

  // get the first and last IRs used
  // (could be easier to find out if we had a ROFRecord of some sort)
  uint32_t firstOrbit = std::numeric_limits<uint32_t>::max();
  uint32_t lastOrbit = 0;

  for (auto d : (*digits)) {
    o2::InteractionTimeRecord ir(d.getTimeStamp());
    firstOrbit = std::min(firstOrbit, ir.orbit);
    lastOrbit = std::max(lastOrbit, ir.orbit);
  }

  // builds a list of digits per HBF
  std::map<o2::InteractionRecord, std::vector<o2::mch::Digit>> digitsPerIR;

  // start with empty HBFs
  uint16_t bc{0};
  double ts{0};

  for (auto orbit = firstOrbit; orbit <= lastOrbit; orbit++) {
    o2::InteractionTimeRecord ir(o2::InteractionRecord{bc, orbit}, ts);
    digitsPerIR[ir] = {};
  }

  // now add actual digits to the relevant HBFs
  for (auto d : (*digits)) {
    o2::InteractionTimeRecord ir(d.getTimeStamp());
    // get the closest (previous) HBF from this IR
    // and assign the digits to that one
    auto hbf = hbfutils.getIRHBF(hbfutils.getHBF(ir));
    // std::cout << fmt::format("IR {} HBF {}\n",
    //                          asString(ir), asString(hbf));
    digitsPerIR[hbf].push_back(d);
  }

  // rescale the timetamp of each digit to be relative to BC
  // FIXME
  int n{0};
  int ndigits{0};
  std::set<uint16_t> orbits;
  for (auto p : digitsPerIR) {

    auto h = fmt::format("{:4}/{:4d} ORBIT {:6d} BC {:4d} nofDigits {:5d} TS {:g}", n, digitsPerIR.size(),
                         p.first.orbit, p.first.bc, p.second.size(),
                         p.first.bc2ns());
    orbits.insert(p.first.orbit);
    std::cout << h << "\n";
    ndigits += p.second.size();
    // for (auto& d : p.second) {
    //   auto ts = static_cast<int>(std::abs(d.getTimeStamp() - p.first.bc2ns()));
    //   d = o2::mch::Digit(ts, d.getDetID(), d.getPadID(), d.getADC());
    //   std::cout << h << " " << d << "\n";
    // }
    n++;
  }

  std::cout << fmt::format("{:6d} digits {:4d} orbits {:6d} bcs\n",
                           ndigits, orbits.size(), digitsPerIR.size());
  if (dummyElecMap) {
    std::cout << "WARNING: using dummy electronic mapping\n";
  }
  auto det2elec = (dummyElecMap ? createDet2ElecMapper<ElectronicMapperDummy>(deIdsForAllMCH) : createDet2ElecMapper<ElectronicMapperGenerated>(deIdsOfCH5L /*deIdsForAllMCH*/));
  auto solar2cru = (dummyElecMap ? createSolar2CruLinkMapper<ElectronicMapperDummy>() : createSolar2CruLinkMapper<ElectronicMapperGenerated>());

  DigitEncoder encoder = createDigitEncoder(userLogic, det2elec);

  std::ofstream out(output);

  digit2raw<o2::header::RAWDataHeaderV4>(digitsPerIR, encoder, solar2cru, out);
  return 0;
}
