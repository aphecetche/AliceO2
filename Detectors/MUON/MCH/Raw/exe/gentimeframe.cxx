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
#include "DumpBuffer.h"
#include "Headers/RAWDataHeader.h"
#include "MCHMappingFactory/CreateSegmentation.h"
#include "MCHRawCommon/DataFormats.h"
#include "MCHRawCommon/RDHManip.h"
#include "MCHRawCommon/SampaCluster.h"
#include "MCHRawElecMap/Mapper.h"
#include "MCHRawEncoder/Encoder.h"
#include "MCHRawEncoder/Paginator.h"
#include "MCHSimulation/Digit.h"
#include "Steer/InteractionSampler.h"
#include "gtfGenerateDigits.h"
#include <array>
#include <boost/program_options.hpp>
#include <fmt/format.h>
#include <fmt/printf.h>
#include <fstream>
#include <gsl/span>
#include <iostream>
#include <map>
#include <vector>

using namespace o2::mch;
using namespace o2::mch::raw;
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

template <typename CHARGESUM>
std::vector<SampaCluster> createSampaClusters(uint16_t ts, float adc);

template <>
std::vector<SampaCluster> createSampaClusters<raw::ChargeSumMode>(uint16_t ts, float adc)
{
  return {raw::SampaCluster(ts, static_cast<uint32_t>(adc))};
}

template <>
std::vector<SampaCluster> createSampaClusters<raw::SampleMode>(uint16_t ts, float adc)
{
  uint32_t a = static_cast<uint32_t>(adc);
  std::vector<uint16_t> samples;

  samples.emplace_back(static_cast<uint16_t>((a & 0xFFC00) >> 10));
  samples.emplace_back(static_cast<uint16_t>(a & 0x3FF));
  return {raw::SampaCluster(ts, samples)};
}

template <typename FORMAT, typename CHARGESUM, typename RDH>
void encodeDigits(gsl::span<o2::mch::Digit> digits,
                  std::unique_ptr<Encoder>& encoder,
                  std::function<std::optional<DsElecId>(DsDetId)> det2elec,
                  uint32_t orbit,
                  uint16_t bc)

{
  bool startHB{false};

  static int nadd{0};

  for (auto d : digits) {
    int deid = d.getDetID();
    int dsid = mapping::segmentation(deid).padDualSampaId(d.getPadID());
    DsDetId detId{deid, dsid};
    auto dselocopt = det2elec(DsDetId(deid, dsid));
    if (!dselocopt.has_value()) {
      std::cout << fmt::format("WARNING : got no location for (de,ds)=({},{})\n", deid, dsid);
      continue;
    }
    DsElecId elecId = dselocopt.value();
    if (!startHB) {
      encoder->startHeartbeatFrame(orbit, bc);
      startHB = true;
    }
    int dschid = mapping::segmentation(deid).padDualSampaChannel(d.getPadID());
    uint16_t ts(666); // FIXME: simulate something here ?
    int sampachid = dschid % 32;

    auto clusters = createSampaClusters<CHARGESUM>(ts, d.getADC());

    fmt::printf(
      "nadd %6d deid %4d dsid %4d dschid %2d "
      "solarId %3d groupId %2d elinkId %2d sampach %2d\n",
      nadd++, deid, dsid, dschid,
      elecId.solarId(), elecId.elinkGroupId(), elecId.elinkId(), sampachid);
    std::cout << clusters[0] << "\n";

    encoder->addChannelData(elecId, sampachid, clusters);
  }
}

template <typename FORMAT, typename CHARGESUM, typename RDH>
void encode(gsl::span<o2::InteractionTimeRecord> interactions,
            gsl::span<std::vector<o2::mch::Digit>> digitsPerInteraction,
            std::function<std::optional<CruLinkId>(uint16_t)> solar2cruLink,
            std::function<std::optional<DsElecId>(DsDetId)> det2elec,
            std::vector<uint8_t>& buffer)
{
  auto encoder = createEncoder<FORMAT, CHARGESUM, RDH>(solar2cruLink);

  for (int i = 0; i < interactions.size(); i++) {

    auto& col = interactions[i];

    std::cout << fmt::format(">>> BEGIN INTERACTION {:4d} ", i) << col << fmt::format(" {:4d} digits\n", digitsPerInteraction[i].size());

    encodeDigits<FORMAT, CHARGESUM, RDH>(digitsPerInteraction[i],
                                         encoder, det2elec, col.orbit, col.bc);

    encoder->moveToBuffer(buffer);

    std::cout << fmt::format("<<< END INTERACTION {:4d} buffer size {}\n", i, buffer.size());
  }
}

template <typename FORMAT, typename CHARGESUM, typename RDH, typename ELECMAP = o2::mch::raw::ElectronicMapperDummy>
void gentimeframe(std::ostream& outfile, const int nofInteractionsPerTimeFrame)
{
  std::cout << __PRETTY_FUNCTION__ << "\n";
  o2::steer::InteractionSampler sampler; // default sampler with default filling scheme, rate of 50 kHz

  constexpr float occupancy = 1E-3;

  std::vector<o2::InteractionTimeRecord> interactions = getCollisions(sampler, nofInteractionsPerTimeFrame);

  // auto allDeIds = getAllDetectionElementIds();
  auto deIds = deIdsOfCH5L;

  // one vector of digits per interaction
  // std::vector<std::vector<o2::mch::Digit>> digitsPerInteraction = generateDigits(interactions.capacity(), deids, occupancy);
  std::vector<std::vector<o2::mch::Digit>> digitsPerInteraction = generateFixedDigits(interactions.capacity(), deIds);

  std::vector<uint8_t> buffer;
  encode<FORMAT, CHARGESUM, RDH>(interactions, digitsPerInteraction,
                                 createSolar2CruLinkMapper<ELECMAP>(),
                                 createDet2ElecMapper<ELECMAP>(deIds),
                                 buffer);

  if (buffer.empty()) {
    std::cout << "Something went wrong : got an empty buffer\n";
    return;
  }

  std::cout << fmt::format("output buffer is {:5.2f} MB\n", 1.0 * buffer.size() / 1024 / 1024);

#if 0
  o2::mch::raw::impl::dumpBuffer(buffer);
  std::cout << "-------------------- Paginating ...\n";
  std::vector<uint8_t> pages;
  size_t pageSize = 8192;
  uint8_t paddingByte = 0x44;
  gsl::span<uint8_t> b8(buffer);
  raw::paginateBuffer<RDH>(b8, pages, pageSize, paddingByte);
  outfile.write(reinterpret_cast<char*>(&pages[0]), pages.size());
#endif
  outfile.write(reinterpret_cast<char*>(&buffer[0]), buffer.size());

  auto n = o2::mch::raw::countRDHs<RDH>(buffer);

  std::cout << "n=" << n << " collisions=" << interactions.size() << "\n";
}

int main(int argc, char* argv[])
{
  po::options_description generic("options");
  bool userLogic{false};
  bool chargeSum{false};
  bool dummyElecMap{false};
  std::string filename;
  po::variables_map vm;
  int nofInteractionsPerTimeFrame{1000};

  // clang-format off
  generic.add_options()
      ("help:h", "produce help message")
      ("userLogic,u",po::bool_switch(&userLogic),"user logic format")
      ("chargesum,c",po::bool_switch(&chargeSum),"charge sum mode")
      ("dummyElecMap,d",po::bool_switch(&dummyElecMap),"use a dummy electronic mapping (for testing only)")
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

  if (dummyElecMap) {
    if (userLogic) {
      if (chargeSum) {
        gentimeframe<raw::UserLogicFormat, raw::ChargeSumMode, o2::header::RAWDataHeaderV4, raw::ElectronicMapperDummy>(outfile, nofInteractionsPerTimeFrame);
      } else {
        gentimeframe<raw::UserLogicFormat, raw::SampleMode, o2::header::RAWDataHeaderV4, raw::ElectronicMapperDummy>(outfile, nofInteractionsPerTimeFrame);
      }
    }
    if (!userLogic) {
      if (chargeSum) {
        gentimeframe<raw::BareFormat, raw::ChargeSumMode, o2::header::RAWDataHeaderV4, raw::ElectronicMapperDummy>(outfile, nofInteractionsPerTimeFrame);
      } else {
        gentimeframe<raw::BareFormat, raw::SampleMode, o2::header::RAWDataHeaderV4, raw::ElectronicMapperDummy>(outfile, nofInteractionsPerTimeFrame);
      }
    }
  } else {
    if (userLogic) {
      if (chargeSum) {
        gentimeframe<raw::UserLogicFormat, raw::ChargeSumMode, o2::header::RAWDataHeaderV4, raw::ElectronicMapperGenerated>(outfile, nofInteractionsPerTimeFrame);
      } else {
        gentimeframe<raw::UserLogicFormat, raw::SampleMode, o2::header::RAWDataHeaderV4, raw::ElectronicMapperGenerated>(outfile, nofInteractionsPerTimeFrame);
      }
    }
    if (!userLogic) {
      if (chargeSum) {
        gentimeframe<raw::BareFormat, raw::ChargeSumMode, o2::header::RAWDataHeaderV4, raw::ElectronicMapperGenerated>(outfile, nofInteractionsPerTimeFrame);
      } else {
        gentimeframe<raw::BareFormat, raw::SampleMode, o2::header::RAWDataHeaderV4, raw::ElectronicMapperGenerated>(outfile, nofInteractionsPerTimeFrame);
      }
    }
  }
  return 0;
}
