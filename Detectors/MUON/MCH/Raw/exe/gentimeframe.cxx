// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

// gentimeframe
//
// helper program to generate raw data for (the equivalent of) a full
// (sub)timeframe (parameter here is a number of DEs for which we generate
// data)
//
// first generate (or get from somewhere) enough digits to "simulate"
// the occupancy of that (sub)timeframe
//
// then convert all those digits into raw data buffer(s)
// (either bare or userlogic format)
//
// write those buffers to disk
//
// have a look at the output sizes and, more importantly, at how long it takes
// to generate them.
//
// have also a look at the time it takes to read back those raw data files
//
//
//
//
// for the digit generation part, parameters :
//
// - number of events
// - mean number of digits per event (-> Poisson distrib ? simple gauss ?)
//
// - (not needed for this project, but might come handy) : mean time between
// two events (or some kind of event time distribution, to be able to simulate
// e.g. pile-up or ...). That part could take as input some vectors of digits
// and (re-)organize them into time buckets depending on some parameters.
// => use InteractionSampler for that ?
//
//

#include "CommonDataFormat/InteractionRecord.h"
#include "Headers/RAWDataHeader.h"
#include "MCHRawCommon/DataFormats.h"
#include "MCHRawCommon/SampaCluster.h"
#include "MCHRawCommon/RDHManip.h"
#include "MCHRawEncoder/ElectronicMapper.h"
#include "MCHRawEncoder/Encoder.h"
#include "MCHRawEncoder/Paginator.h"
#include "MCHSimulation/Digit.h"
#include "Steer/InteractionSampler.h"
#include <array>
#include <fmt/format.h>
#include <fmt/printf.h>
#include <gsl/span>
#include <iostream>
#include <map>
#include <vector>
#include <boost/program_options.hpp>
#include <fstream>
#include "gtfGenerateDigits.h"
#include "DumpBuffer.h"
#include "MCHMappingFactory/CreateSegmentation.h"

using namespace o2::mch;
namespace po = boost::program_options;

std::vector<o2::InteractionTimeRecord> getCollisions(o2::steer::InteractionSampler& sampler,
                                                     int nofCollisions)
{
  std::vector<o2::InteractionTimeRecord> records;
  records.reserve(nofCollisions);
  sampler.init();
  sampler.generateCollisionTimes(records);
  return records;
}

std::vector<int> getAllDetectionElementIds()
{
  std::vector<int> deids;
  mapping::forEachDetectionElement([&](int deid) {
    deids.push_back(deid);
  });
  return deids;
}

template <typename FORMAT, typename CHARGESUM, typename RDH>
void encodeDigits(gsl::span<o2::mch::Digit> digits,
                  std::map<int, std::unique_ptr<raw::CRUEncoder>>& crus,
                  o2::mch::raw::ElectronicMapper& elecmap,
                  uint32_t orbit,
                  uint16_t bc)

{
  std::map<int, bool> startHB;

  static int nadd{0};

  for (auto d : digits) {
    int deid = d.getDetID();
    uint8_t cruId = elecmap.cruId(deid);
    if (cruId == 0xFF) {
      // std::cout << "WARNING : no electronic mapping found for DE " << deid << "\n";
      continue;
    }
    if (crus.find(cruId) == crus.end()) {
      crus[cruId] = raw::createCRUEncoder<FORMAT, CHARGESUM, RDH>(cruId, elecmap);
    }
    auto& cru = crus[cruId];
    if (!cru) {
      continue;
      // std::cout << "No encoder made for CRU " << cruId << "\n";
    }
    if (!startHB[cruId]) {
      cru->startHeartbeatFrame(orbit, bc);
      startHB[cruId] = true;
    }
    int dsid = mapping::segmentation(deid).padDualSampaId(d.getPadID());
    int dschid = mapping::segmentation(deid).padDualSampaChannel(d.getPadID());
    auto dseloc = elecmap.dualSampaElectronicLocation(deid, dsid);
    uint16_t ts(666); // FIXME: simulate something here ?
    int sampachid = dschid % 32;
    uint32_t adc = static_cast<uint32_t>(d.getADC());

    fmt::printf(
      "nadd %6d deid %4d dsid %4d dschid %2d "
      "cruId %2d solarId %3d groupId %2d elinkId %2d sampach %2d "
      "ADC %6d (%4d|%4d)\n",
      nadd++, deid, dsid, dschid,
      cruId, dseloc.solarId(), dseloc.elinkGroupId(), dseloc.elinkId(), sampachid,
      adc, ((adc & 0xFFC00) >> 10), (adc & 0x3FF));

    cru->addChannelData(dseloc.solarId(), dseloc.elinkId(), sampachid, {raw::SampaCluster(ts, adc)});
  }
}

template <typename FORMAT, typename CHARGESUM, typename RDH>
void encode(gsl::span<o2::InteractionTimeRecord> interactions,
            gsl::span<std::vector<o2::mch::Digit>> digitsPerInteraction,
            raw::ElectronicMapper& elecmap,
            std::vector<uint8_t>& buffer)
{
  std::map<int, std::unique_ptr<raw::CRUEncoder>> crus;

  for (int i = 0; i < interactions.size(); i++) {

    auto& col = interactions[i];

    std::cout << fmt::format("BEGIN INTERACTION {:4d} ", i) << col << fmt::format(" {:4d} digits\n", digitsPerInteraction[i].size());

    encodeDigits<FORMAT, CHARGESUM, RDH>(digitsPerInteraction[i],
                                         crus, elecmap, col.orbit, col.bc);

    for (auto& p : crus) {
      if (p.second) {
        p.second->moveToBuffer(buffer);
      }
    }

    std::cout << fmt::format("END INTERACTION {:4d} buffer size {}\n", i, buffer.size());
  }
}

// filter out detection elements for which we don't have the
// electronic mapping
std::vector<int> filterDetectionElements(gsl::span<int> deids,
                                         const raw::ElectronicMapper& elecmap)
{
  std::vector<int> d;
  for (auto deid : deids) {
    if (elecmap.cruId(deid) != 0xFF) {
      d.emplace_back(deid);
    }
  }
  return d;
}

template <typename FORMAT, typename CHARGESUM, typename RDH>
void gentimeframe(std::ostream& outfile, const int nofInteractionsPerTimeFrame)
{
  std::cout << __PRETTY_FUNCTION__ << "\n";
  o2::steer::InteractionSampler sampler; // default sampler with default filling scheme, rate of 50 kHz

  constexpr float occupancy = 1E-3;

  std::vector<o2::InteractionTimeRecord> interactions = getCollisions(sampler, nofInteractionsPerTimeFrame);

  //auto deids = getAllDetectionElementIds();
  auto elecmap = raw::createElectronicMapper<raw::ElectronicMapperGenerated>();
  std::vector<int> ch5l = {505, 506, 507, 508, 509, 510, 511, 512, 513};
  std::vector<int> deids = filterDetectionElements(gsl::span<int>(ch5l), *elecmap);

  // one vector of digits per interaction
  // std::vector<std::vector<o2::mch::Digit>> digitsPerInteraction = generateDigits(interactions.capacity(), deids, occupancy);
  std::vector<std::vector<o2::mch::Digit>> digitsPerInteraction = generateFixedDigits(interactions.capacity(), deids);

  std::vector<uint8_t> buffer;
  encode<FORMAT, CHARGESUM, RDH>(interactions, digitsPerInteraction, *elecmap, buffer);
  std::cout << fmt::format("output buffer is {:5.2f} MB\n", 1.0 * buffer.size() / 1024 / 1024);

  o2::mch::raw::impl::dumpBuffer(buffer);
  std::cout << "-------------------- Paginating ...\n";
  std::vector<uint8_t> pages;
  size_t pageSize = 8192;
  uint8_t paddingByte = 0x44;
  gsl::span<uint8_t> b8(buffer);
  raw::paginateBuffer<RDH>(b8, pages, pageSize, paddingByte);
  outfile.write(reinterpret_cast<char*>(&pages[0]), pages.size());

  auto n = o2::mch::raw::countRDHs<RDH>(buffer);

  std::cout << "n=" << n << " collisions=" << interactions.size() << "\n";
}

int main(int argc, char* argv[])
{
  po::options_description generic("options");
  bool userLogic{false};
  std::string filename;
  po::variables_map vm;
  int nofInteractionsPerTimeFrame{1000};

  // clang-format off
  generic.add_options()
      ("help:h", "produce help message")
      ("userLogic,u",po::bool_switch(&userLogic),"user logic format")
      ("outfile,o",po::value<std::string>(&filename)->required(),"output filename")
      ("nintpertf,n",po::value<int>(&nofInteractionsPerTimeFrame),"number of interactions per timeframe")
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

  std::ofstream outfile(filename);

  if (userLogic) {
    gentimeframe<raw::UserLogicFormat, raw::ChargeSumMode, o2::header::RAWDataHeaderV4>(outfile, nofInteractionsPerTimeFrame);
  }
  if (!userLogic) {
    gentimeframe<raw::BareFormat, raw::ChargeSumMode, o2::header::RAWDataHeaderV4>(outfile, nofInteractionsPerTimeFrame);
  }
  return 0;
}
