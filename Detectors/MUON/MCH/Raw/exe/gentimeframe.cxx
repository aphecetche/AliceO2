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
#include "gtfSegmentation.h"
#include "DumpBuffer.h"

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

  for (auto d : digits) {
    int deid = d.getDetID();
    uint8_t cruId = elecmap.cruId(deid);
    if (cruId == 0xFF) {
      // std::cout << "WARNING : no electronic mapping found for DE " << deid << "\n";
      continue;
    }
    if (crus.find(cruId) == crus.end()) {
      crus[cruId] = raw::createCRUEncoder<FORMAT, CHARGESUM, RDH>(cruId, elecmap);
      fmt::printf("Create cru %d\n", cruId);
    }
    auto& cru = crus[cruId];
    if (!cru) {
      continue;
      // std::cout << "No encoder made for CRU " << cruId << "\n";
    }
    if (!startHB[cruId]) {
      cru->startHeartbeatFrame(orbit, bc);
      fmt::printf("startHB for CRU %d orbit %d bc %d\n", cruId, orbit, bc);
      startHB[cruId] = true;
    }
    int dsid = segmentation(deid).padDualSampaId(d.getPadID());
    int chid = segmentation(deid).padDualSampaChannel(d.getPadID());
    auto p = elecmap.solarIdAndGroupIdFromDeIdAndDsId(deid, dsid);
    uint16_t solarId = p.first;
    uint16_t elinkId = p.second;
    uint16_t ts(0); // FIXME: simulate something here ?
    // fmt::printf("nadd %8d cruid %2d deid %4d dsid %4d solarId %3d elinkId %2d chId %2d ADC %4d\n", nadd++, cruId,
    //             deid, dsid, solarId, elinkId, chid, static_cast<uint16_t>(d.getADC()));
    cru->addChannelData(solarId, elinkId, chid % 32, {raw::SampaCluster(ts, static_cast<uint16_t>(d.getADC()))});
  }
}

template <typename FORMAT, typename CHARGESUM, typename RDH>
void encode(gsl::span<o2::InteractionTimeRecord> interactions,
            gsl::span<std::vector<o2::mch::Digit>> digitsPerInteraction,
            std::vector<uint8_t>& buffer)
{
  auto elecmap = raw::createElectronicMapper<raw::ElectronicMapperGenerated>();
  std::map<int, std::unique_ptr<raw::CRUEncoder>> crus;

  int nadd{0};

  for (int i = 0; i < interactions.size(); i++) {

    auto& col = interactions[i];

    std::cout << "INTERACTION " << col << "\n";

    encodeDigits<FORMAT, CHARGESUM, RDH>(digitsPerInteraction[i],
                                         crus, *elecmap, col.orbit, col.bc);

    for (auto& p : crus) {
      if (p.second) {
        p.second->moveToBuffer(buffer);
      }
    }

    std::cout << fmt::format("interaction {} buffer size {}\n", i, buffer.size());
  }
}

template <typename FORMAT, typename CHARGESUM, typename RDH>
void gentimeframe(std::ostream& outfile)
{
  o2::steer::InteractionSampler sampler; // default sampler with default filling scheme, rate of 50 kHz

  constexpr int nofInteractionsPerTimeFrame{10};
  constexpr float occupancy = 1E-3;

  std::vector<o2::InteractionTimeRecord> interactions = getCollisions(sampler, nofInteractionsPerTimeFrame);

  //auto deids = getAllDetectionElementIds();
  std::vector<int> deids = {505, 506, 507, 508, 509, 510, 511, 512, 513}; // CH5L

  // one vector of digits per interaction
  std::vector<std::vector<o2::mch::Digit>> digitsPerInteraction = generateDigits(interactions.capacity(), deids, occupancy);

  std::vector<uint8_t> buffer;
  encode<FORMAT, CHARGESUM, RDH>(interactions, digitsPerInteraction, buffer);
  std::cout << fmt::format("output buffer is {:5.2f} MB\n", 1.0 * buffer.size() / 1024 / 1024);

  std::cout << "--------------------\n";
  o2::mch::raw::showRDHs<RDH>(buffer);

  o2::mch::raw::impl::dumpBuffer(buffer);

  gsl::span<uint32_t> buffer32(reinterpret_cast<uint32_t*>(&buffer[0]),
                               buffer.size() / 4);
  o2::mch::raw::dumpRDHBuffer(buffer32, "");

  std::cout << "-------------------- Paginating ...\n";
  std::vector<uint8_t> pages;
  size_t pageSize = 8192;
  uint8_t paddingByte = 0x0;
  gsl::span<uint8_t> b8(&buffer[0], buffer.size());
  raw::paginateBuffer<RDH>(b8, pages, pageSize, paddingByte);
  // outfile.write(reinterpret_cast<char*>(&pages[0]), pages.size());
}

int main(int argc, char* argv[])
{
  po::options_description generic("options");
  bool userLogic{false};
  std::string filename;
  po::variables_map vm;

  // clang-format off
  generic.add_options()
      ("help:h", "produce help message")
      ("userLogic,u",po::bool_switch(&userLogic),"user logic format")
      ("outfile,o",po::value<std::string>(&filename)->required(),"output filename")
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
    gentimeframe<raw::UserLogicFormat, raw::ChargeSumMode, o2::header::RAWDataHeaderV4>(outfile);
  }
  if (!userLogic) {
    gentimeframe<raw::BareFormat, raw::ChargeSumMode, o2::header::RAWDataHeaderV4>(outfile);
  }
  return 0;
}
