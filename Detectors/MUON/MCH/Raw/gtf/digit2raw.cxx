// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "CommonDataFormat/InteractionRecord.h"
#include "DetectorsRaw/HBFUtils.h"
#include "Digit2RawConverter.h"
#include "Headers/RAWDataHeader.h"
#include "MCHBase/Digit.h"
#include "MCHRawCommon/DataFormats.h"
#include "MCHRawElecMap/Mapper.h"
#include "MCHRawEncoder/DigitEncoder.h"
#include <TBranch.h>
#include <TFile.h>
#include <TTree.h>
#include <boost/program_options.hpp>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <limits>
#include <rapidjson/document.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <string>

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

std::function<std::set<int>(int deId)> createDualSampaMapper();

std::set<uint16_t> getFeeIds(std::function<std::optional<DsElecId>(DsDetId)> det2elec)
{
  std::set<uint16_t> feeIds;
  auto dslist = o2::mch::raw::createDualSampaMapper();
  for (auto deId : deIdsForAllMCH) {
    auto ds = dslist(deId);
    for (auto d : ds) {
      DsDetId detId(deId, d);
      auto dsElecId = det2elec(detId);
      if (!dsElecId.has_value()) {
        throw std::logic_error(fmt::format("could not get dsElecId for dsDetId={}\n", asString(detId)));
      }
      feeIds.insert(dsElecId->solarId());
    }
  }
  return feeIds;
}

std::string digitIdAsString(const o2::mch::Digit& digit,
                            const Digit2ElecMapper& digit2elec)
{
  auto optElecId = digit2elec(digit);
  if (!optElecId.has_value()) {
    return "UNKNOWN";
  }
  auto dsElecId = optElecId.value().first;
  auto dschid = optElecId.value().second;
  return fmt::format("{}-CH{}", asString(dsElecId), dschid);
}

void outputToJson(const std::vector<o2::mch::Digit>& digits,
                  std::function<std::optional<DsElecId>(DsDetId)> det2elec)

{
  rapidjson::OStreamWrapper osw(std::cout);
  rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);

  auto digit2elec = createDigit2ElecMapper(det2elec);

  writer.StartObject();
  writer.Key("digits");
  writer.StartArray();
  for (auto d : digits) {
    writer.StartObject();
    writer.Key("id");
    auto sid = digitIdAsString(d, digit2elec);
    writer.String(sid.c_str());
    writer.Key("adc");
    writer.Int(d.getADC());
    writer.EndObject();
  }
  writer.EndArray();
  writer.EndObject();
}

int main(int argc, char* argv[])
{
  po::options_description generic("options");
  bool userLogic{false};
  bool dummyElecMap{false};
  std::string input;
  std::string output;
  po::variables_map vm;
  bool jsonOutput;

  // clang-format off
  generic.add_options()
      ("help,h", "produce help message")
      ("userLogic,u",po::bool_switch(&userLogic),"user logic format")
      ("dummyElecMap,d",po::bool_switch(&dummyElecMap),"use a dummy electronic mapping (for testing only)")
      ("outfile,o",po::value<std::string>(&output)->required(),"output filename")
      ("infile,i",po::value<std::string>(&input)->required(),"input file name")
      ("json,j",po::bool_switch(&jsonOutput),"output means and rms in json format");
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
    digitsPerIR[hbf].push_back(d);
  }

  int ndigits{0};
  for (auto p : digitsPerIR) {
    ndigits += p.second.size();
  }

  if (dummyElecMap && !jsonOutput) {
    std::cout << "WARNING: using dummy electronic mapping\n";
  }
  auto det2elec = (dummyElecMap ? createDet2ElecMapper<ElectronicMapperDummy>(deIdsForAllMCH) : createDet2ElecMapper<ElectronicMapperGenerated>(deIdsOfCH5L /*deIdsForAllMCH*/));
  auto solar2cru = (dummyElecMap ? createSolar2CruLinkMapper<ElectronicMapperDummy>() : createSolar2CruLinkMapper<ElectronicMapperGenerated>());

  if (jsonOutput) {
    outputToJson(*digits, det2elec);
  } else {
    std::cout << fmt::format("{:6d} digits {:4d} orbits\n", ndigits, digitsPerIR.size());
  }

  auto feeIds = getFeeIds(det2elec);

  DigitEncoder encoder = createDigitEncoder(userLogic, det2elec);

  std::ofstream out(output);

  digit2raw<o2::header::RAWDataHeaderV4>(digitsPerIR, feeIds, encoder, solar2cru, out);

  return 0;
}
