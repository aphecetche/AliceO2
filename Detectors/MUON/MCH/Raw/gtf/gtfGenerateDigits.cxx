// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "gtfGenerateDigits.h"
#include <random>
#include "MCHMappingFactory/CreateSegmentation.h"
#include "Steer/InteractionSampler.h"

// generate n digits randomly distributed over the detection elements
// whose ids are within the deids span
std::vector<o2::mch::Digit> makeNRandomDigits(int n, gsl::span<int> deids, gsl::span<int> nofpads)
{
  std::vector<o2::mch::Digit> digits;
  // static std::random_device rd;
  // static std::mt19937 gen(rd());
  static std::mt19937 gen(1234);
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

std::vector<o2::mch::Digit> generateFixedDigits(gsl::span<int> deids)
{
  std::vector<o2::mch::Digit> digits;

  for (auto deid : deids) {
    int n = deid % 100 + 1;
    for (auto j = 0; j < n; j++) {
      int padid = j;
      double adc = ((j * 2) << 10 | (j * 2));
      double time = 987;
      digits.emplace_back(time, deid, padid, adc);
    }
  }
  return digits;
}

/// generate fake digits
/// each DE gets n digits (where n = (DEID%100)+1), with padids ranging
/// from 0 to n-1
/// each digit has a fixed time of 987 and an adc value = padid*2 << 10 | padid*2
std::vector<o2::mch::Digit> generateRandomDigits(gsl::span<int> deids, float occupancy)
{
  std::vector<o2::mch::Digit> digits;
  // std::random_device rd;
  // std::mt19937 gen(rd());
  std::mt19937 gen(1234);

  std::vector<int> nofPads;
  int totalPads{0};
  for (auto d : deids) {
    auto npads = o2::mch::mapping::segmentation(d).nofPads();
    nofPads.emplace_back(npads);
    totalPads += npads;
  }

  std::poisson_distribution<> dis(static_cast<int>(occupancy * totalPads));

  // draw n from mu
  int n = static_cast<int>(dis(gen));
  // generate n random digits
  return makeNRandomDigits(n, deids, gsl::make_span(nofPads));
}

std::vector<o2::InteractionTimeRecord> getCollisions(int nofCollisions)
{
  o2::steer::InteractionSampler sampler; // default sampler with default filling scheme, rate of 50 kHz
  //sampler.setInteractionRate(50000);
  std::vector<o2::InteractionTimeRecord> records;
  records.reserve(nofCollisions);
  sampler.init();
  sampler.generateCollisionTimes(records);
  return records;
}

std::map<o2::InteractionTimeRecord, std::vector<o2::mch::Digit>> generateDigits(
  int nofInteractionsPerTimeFrame,
  bool fixed)
{
  constexpr float occupancy = 1E-3;

  std::vector<o2::InteractionTimeRecord> interactions = getCollisions(nofInteractionsPerTimeFrame);

  //auto deIds = deIdsOfCH5L; //deIdsForAllMCH
  std::array<int, 2> deIds = {500, 501};
  std::map<o2::InteractionTimeRecord, std::vector<o2::mch::Digit>> digitsPerIR;

  for (auto ir : interactions) {
    digitsPerIR[ir] = (fixed ? generateFixedDigits(deIds) : generateRandomDigits(deIds, occupancy));
  }
  return digitsPerIR;
}
