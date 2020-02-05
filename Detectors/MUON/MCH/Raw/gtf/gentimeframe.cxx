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
#include "DetectorsRaw/HBFUtils.h"
#include "DumpBuffer.h"
#include "Headers/RAWDataHeader.h"
#include "MCHBase/Digit.h"
#include "MCHMappingFactory/CreateSegmentation.h"
#include "MCHRawCommon/DataFormats.h"
#include "MCHRawCommon/RDHManip.h"
#include "MCHRawCommon/SampaCluster.h"
#include "MCHRawElecMap/Mapper.h"
#include "MCHRawEncoder/CruLinkSetter.h"
#include "MCHRawEncoder/Encoder.h"
#include "MCHRawEncoder/Normalizer.h"
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

using namespace o2::mch::mapping;
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

template <typename FORMAT, typename CHARGESUM>
void encodeDigits(gsl::span<o2::mch::Digit> digits,
                  std::unique_ptr<Encoder>& encoder,
                  std::function<std::optional<DsElecId>(DsDetId)> det2elec,
                  uint32_t orbit,
                  uint16_t bc)

{
  static int nadd{0};

  for (auto d : digits) {
    int deid = d.getDetID();
    int dsid = segmentation(deid).padDualSampaId(d.getPadID());
    DsDetId detId{deid, dsid};
    auto dselocopt = det2elec(DsDetId(deid, dsid));
    if (!dselocopt.has_value()) {
      std::cout << fmt::format("WARNING : got no location for (de,ds)=({},{})\n", deid, dsid);
      continue;
    }
    DsElecId elecId = dselocopt.value();
    int dschid = segmentation(deid).padDualSampaChannel(d.getPadID());
    uint16_t ts(static_cast<int>(d.getTimeStamp()) & 0xFFFF);
    int sampachid = dschid % 32;

    auto clusters = createSampaClusters<CHARGESUM>(ts, d.getADC());

    fmt::printf(
      "nadd %6d deid %4d dsid %4d dschid %2d "
      "solarId %3d groupId %2d elinkId %2d sampach %2d : %s\n",
      nadd++, deid, dsid, dschid,
      elecId.solarId(), elecId.elinkGroupId(), elecId.elinkId(), sampachid,
      asString(clusters[0]).c_str());

    encoder->addChannelData(elecId, sampachid, clusters);
  }
}

template <typename FORMAT, typename CHARGESUM>
void encode(gsl::span<o2::InteractionTimeRecord> interactions,
            gsl::span<std::vector<o2::mch::Digit>> digitsPerInteraction,
            std::function<std::optional<DsElecId>(DsDetId)> det2elec,
            std::vector<uint8_t>& buffer)
{
  auto encoder = createEncoder<FORMAT, CHARGESUM>();

  for (int i = 0; i < interactions.size(); i++) {
    auto& col = interactions[i];
    encoder->startHeartbeatFrame(col.orbit, col.bc);
    encodeDigits<FORMAT, CHARGESUM>(digitsPerInteraction[i],
                                    encoder, det2elec, col.orbit, col.bc);
    encoder->moveToBuffer(buffer);
  }
}

template <typename FORMAT,
          typename CHARGESUM,
          typename RDH,
          typename ELECMAP = ElectronicMapperGenerated>
void gentimeframe(std::ostream& outfile, const int nofInteractionsPerTimeFrame)
{
  std::cout << __PRETTY_FUNCTION__ << "\n";
  o2::steer::InteractionSampler sampler; // default sampler with default filling scheme, rate of 50 kHz
  sampler.setInteractionRate(1000);
  //sampler.setInteractionRate(4000000);

  constexpr float occupancy = 1E-3;

  std::vector<o2::InteractionTimeRecord> interactions = getCollisions(sampler, nofInteractionsPerTimeFrame);

  //auto deIds = deIdsOfCH5L; //deIdsForAllMCH
  std::array<int, 2> deIds = {500, 501};

  // one vector of digits per interaction
  // std::vector<std::vector<o2::mch::Digit>> digitsPerInteraction = generateDigits(interactions.capacity(), deids, occupancy);
  std::vector<std::vector<o2::mch::Digit>> digitsPerInteraction = generateFixedDigits(interactions.capacity(), deIds);

  std::vector<uint8_t> buffer;
  encode<FORMAT, CHARGESUM>(interactions, digitsPerInteraction,
                            createDet2ElecMapper<ELECMAP>(deIds),
                            buffer);

  if (buffer.empty()) {
    std::cout << "Something went wrong : got an empty buffer\n";
    return;
  }
  std::vector<uint8_t> outBuffer;
  normalizeBuffer<RDH>(buffer, outBuffer, interactions);

  auto solar2cru = createSolar2CruLinkMapper<ELECMAP>();
  assignCruLink<RDH>(outBuffer, solar2cru);

  std::cout << "-------------------- SuperPaginating (to be written)\n";
  // FIXME: to be written...
  // superPaginate(buffer);
  outfile.write(reinterpret_cast<char*>(&outBuffer[0]), outBuffer.size());
  std::cout << fmt::format("output buffer is {} bytes ({:5.2f} MB)\n", outBuffer.size(), 1.0 * outBuffer.size() / 1024 / 1024);

  auto n = countRDHs<RDH>(buffer);

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
      ("help,h", "produce help message")
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

  std::ofstream outfile(filename);

  if (dummyElecMap) {
    if (userLogic) {
      if (chargeSum) {
        gentimeframe<UserLogicFormat, ChargeSumMode, o2::header::RAWDataHeaderV4, ElectronicMapperDummy>(outfile, nofInteractionsPerTimeFrame);
      } else {
        gentimeframe<UserLogicFormat, SampleMode, o2::header::RAWDataHeaderV4, ElectronicMapperDummy>(outfile, nofInteractionsPerTimeFrame);
      }
    }
    if (!userLogic) {
      if (chargeSum) {
        gentimeframe<BareFormat, ChargeSumMode, o2::header::RAWDataHeaderV4, ElectronicMapperDummy>(outfile, nofInteractionsPerTimeFrame);
      } else {
        gentimeframe<BareFormat, SampleMode, o2::header::RAWDataHeaderV4, ElectronicMapperDummy>(outfile, nofInteractionsPerTimeFrame);
      }
    }
  } else {
    if (userLogic) {
      if (chargeSum) {
        gentimeframe<UserLogicFormat, ChargeSumMode, o2::header::RAWDataHeaderV4, ElectronicMapperGenerated>(outfile, nofInteractionsPerTimeFrame);
      } else {
        gentimeframe<UserLogicFormat, SampleMode, o2::header::RAWDataHeaderV4, ElectronicMapperGenerated>(outfile, nofInteractionsPerTimeFrame);
      }
    }
    if (!userLogic) {
      if (chargeSum) {
        gentimeframe<BareFormat, ChargeSumMode, o2::header::RAWDataHeaderV4, ElectronicMapperGenerated>(outfile, nofInteractionsPerTimeFrame);
      } else {
        gentimeframe<BareFormat, SampleMode, o2::header::RAWDataHeaderV4, ElectronicMapperGenerated>(outfile, nofInteractionsPerTimeFrame);
      }
    }
  }
  return 0;
}
