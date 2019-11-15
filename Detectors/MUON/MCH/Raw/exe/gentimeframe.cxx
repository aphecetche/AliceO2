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
#include "MCHMappingInterface/Segmentation.h"
#include "MCHSimulation/Digit.h"
#include "MCHRawEncoder/Encoder.h"
#include "MCHRawCommon/DataFormats.h"
#include "MCHRawCommon/SampaCluster.h"
#include "Steer/InteractionSampler.h"
#include <array>
#include <fmt/format.h>
#include <fmt/printf.h>
#include <iostream>
#include <gsl/span>
#include <random>
#include <vector>
#include <map>

extern std::map<int, int> toElec();
extern uint32_t code(uint16_t, uint16_t);

using namespace o2::mch;

using MCHDigit = Digit;

std::vector<o2::InteractionTimeRecord> getCollisions(o2::steer::InteractionSampler& sampler,
                                                     int nofCollisions)
{
  std::vector<o2::InteractionTimeRecord> records;
  records.reserve(nofCollisions);
  sampler.init();
  sampler.generateCollisionTimes(records);
  return records;
}

// generate n digits randomly distributed over the detection elements
// whose ids are within the deids span
std::vector<MCHDigit> generateDigits(int n, gsl::span<int> deids, gsl::span<int> nofpads)
{
  std::vector<MCHDigit> digits;
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<> disDeId(0, deids.size() - 1);
  // std::uniform_int_distribution<> disADC(0, 1023);

  assert(deids.size() == nofpads.size());
  for (auto i = 0; i < n; i++) {
    int j = disDeId(gen);
    int deid = deids[j];
    std::uniform_int_distribution<> np(0, nofpads[j] - 1);
    int padid = np(gen);
    // double adc = disADC(gen);
    double adc = padid;
    double time = 0;
    digits.emplace_back(time, deid, padid, adc);
  }
  return digits;
}

std::vector<int> getAllDetectionElementIds()
{
  std::vector<int> deids;
  mapping::forEachDetectionElement([&](int deid) {
    deids.push_back(deid);
  });
  return deids;
}

std::vector<std::vector<MCHDigit>> getDigits(int nofEvents, gsl::span<int> deids, float occupancy)
{
  std::vector<std::vector<MCHDigit>> digitsPerEvent;
  digitsPerEvent.reserve(nofEvents); // one vector of digits per event
  std::random_device rd;
  std::mt19937 gen(rd());

  std::vector<int> nofPads;
  int totalPads{0};
  for (auto d : deids) {
    mapping::Segmentation seg(d);
    nofPads.emplace_back(seg.nofPads());
    totalPads += seg.nofPads();
  }

  std::poisson_distribution<> dis(static_cast<int>(occupancy * totalPads));

  for (auto i = 0; i < nofEvents; i++) {
    // draw n from mu
    int n = static_cast<int>(dis(gen));
    // generate n random digits
    auto digits = generateDigits(n, deids, gsl::make_span(nofPads));
    // add those n digits into this event
    digitsPerEvent.push_back(digits);
  }
  return digitsPerEvent;
}

std::ostream& operator<<(std::ostream& os, const MCHDigit& d)
{
  os << fmt::format("DE {:4d} PAD {:6d} ADC {:g}\n",
                    d.getDetID(), d.getPadID(), d.getADC());
  return os;
}

void encode(gsl::span<o2::InteractionTimeRecord> interactions,
            gsl::span<std::vector<MCHDigit>> digitsPerInteraction,
            std::vector<uint8_t>& buffer)
{
  uint8_t cruId{0}; // FIXME: get this from digit (deid,padid)=>(cruid,solarid,dsid,chid)

  // auto cru = raw::createCRUEncoderNoPhase<raw::UserLogicFormat, raw::ChargeSumMode>(cruId);
  auto cru = raw::createCRUEncoderNoPhase<raw::BareFormat, raw::ChargeSumMode>(cruId);
  uint16_t ts(0);

  uint16_t chId(0); // FIXME: get this from digit  "

  std::map<int, int> toelec = toElec();
  int nadd{0};
  for (int i = 0; i < interactions.size(); i++) {

    auto& col = interactions[i];

    cru->startHeartbeatFrame(col.orbit, col.bc);

    std::cout << "INTERACTION " << col << "\n";

    for (auto d : digitsPerInteraction[i]) {
      int deid = d.getDetID();
      int dsid = 1; // FIXME get dsid from deid,padid
      uint32_t m = toelec[code(deid, dsid)];
      uint8_t solarId = ((m & 0xFFFF0000) >> 16) & 0xFF;
      uint8_t elinkId = ((m & 0xFFFF)) & 0xFF;
      fmt::printf("nadd %d deid %d dsid %d solarId %d elinkId %d chId %d ADC %d\n", nadd++,
                  deid, dsid, solarId, elinkId, chId, static_cast<uint16_t>(d.getADC()));
      cru->addChannelData(solarId, elinkId, chId, {raw::SampaCluster(ts, static_cast<uint16_t>(d.getADC()))});
    }
    cru->moveToBuffer(buffer);
    std::cout << fmt::format("interaction {} buffer size {}\n", i, buffer.size());
  }
}

int main()
{
  o2::steer::InteractionSampler sampler; // default sampler with default filling scheme, rate of 50 kHz

  constexpr int nofInteractionsPerTimeFrame{1000};
  constexpr float occupancy = 1E-2;

  std::vector<o2::InteractionTimeRecord> interactions = getCollisions(sampler, nofInteractionsPerTimeFrame); // destination for records

  //auto deids = getAllDetectionElementIds();
  std::vector<int> deids = {505, 506, 507, 508, 509, 510, 511, 512, 513}; // CH5L

  // one vector of digits per interaction
  // FIXME: should get one such structure per CRU, i.e. per array of detection elements
  std::vector<std::vector<MCHDigit>> digitsPerInteraction = getDigits(interactions.capacity(), deids, occupancy);

  // encode is supposed to be for 1 CRU only
  std::vector<uint8_t> buffer;
  encode(interactions, digitsPerInteraction, buffer);

  std::cout << fmt::format("output buffer is {} MB\n", buffer.size() / 1024 / 1024);
  return 0;
}
