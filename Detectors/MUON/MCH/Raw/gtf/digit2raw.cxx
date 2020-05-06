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
#include "DetectorsRaw/RawFileWriter.h"
#include "MCHRawEncoder/DataBlock.h"

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
    auto sid = digitIdAsString(d, digit2elec);
    if (sid == "UNKNOWN") {
      continue;
    }
    writer.StartObject();
    writer.Key("id");
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
      ("json,j",po::bool_switch(&jsonOutput),"output digits in json format");
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

  const o2::raw::HBFUtils& hbfutils = o2::raw::HBFUtils::Instance();

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
  auto det2elec = (dummyElecMap ? createDet2ElecMapper<ElectronicMapperDummy>() : createDet2ElecMapper<ElectronicMapperGenerated>());

  if (jsonOutput) {
    outputToJson(*digits, det2elec);
    return 0;
  } else {
    std::cout << fmt::format("{:6d} digits {:4d} orbits\n", ndigits, digitsPerIR.size());
  }

  DigitEncoder encoder = createDigitEncoder(userLogic, det2elec);

  auto solar2feelink = (dummyElecMap ? createSolar2FeeLinkMapper<ElectronicMapperDummy>() : createSolar2FeeLinkMapper<ElectronicMapperGenerated>());

  o2::conf::ConfigurableParam::setValue<uint32_t>("HBFUtils", "orbitFirst", firstOrbit);
  //o2::conf::ConfigurableParam::setValue<uint16_t>("HBFUtils", "bcFirst", F.firstIR.bc);

  o2::raw::RawFileWriter fw;
  fw.setDontFillEmptyHBF(true);

  for (auto p : digitsPerIR) {

    std::vector<std::byte> buffer;

    auto& currentIR = p.first;
    auto& digits = p.second;

    encoder(digits, buffer, currentIR.orbit, currentIR.bc);

    std::set<DataBlockRef> dataBlockRefs;

    forEachDataBlockRef(
      buffer, [&dataBlockRefs](const DataBlockRef& ref) {
        dataBlockRefs.insert(ref);
      });

    // here should only register new links
    {
      std::set<FeeLinkId> feeLinkIds;

      for (auto r : dataBlockRefs) {
        feeLinkIds.insert(solar2feelink(r.block.header.solarId).value());
      }

      for (auto f : feeLinkIds) {
        int endpoint = f.feeId() % 2;
        int cru = (f.feeId() - endpoint) / 2;
        auto& link = fw.registerLink(f.feeId(), cru, f.linkId(), endpoint, "mch.raw");
      }
    }

    for (auto r : dataBlockRefs) {
      auto& b = r.block;
      auto& h = b.header;
      auto f = solar2feelink(r.block.header.solarId).value();
      int endpoint = f.feeId() % 2;
      int cru = (f.feeId() - endpoint) / 2;
      std::cout << fmt::format("feeId {} cruId {} linkId {} endpoint {}\n",
                               f.feeId(), cru, f.linkId(), endpoint);
      fw.addData(f.feeId(), cru, f.linkId(), endpoint, {h.bc, h.orbit}, gsl::span<char>(const_cast<char*>(reinterpret_cast<const char*>(&b.payload[0])), b.payload.size()));
    }
  }

  return 0;
}
