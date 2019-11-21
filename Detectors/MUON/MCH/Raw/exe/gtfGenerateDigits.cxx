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
#include "gtfSegmentation.h"
#include <random>
#include <fmt/printf.h>

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

/// generate fake digits
/// each DE gets n digits (where n = DEID%100), with padids ranging
/// from 0 to n-1
/// each digit has a fixed time of 987 and an adc value = padid*2 << 10 | padid*2
std::vector<std::vector<o2::mch::Digit>> generateFixedDigits(int nofEvents, gsl::span<int> deids)
{
  // auto seg = segmentation(501);
  // auto padid = seg.findPadByFEE(401, 47);
  // fmt::printf("padid %d dsid %d ch %d x %g y %g\n",
  //             padid, seg.padDualSampaId(padid), seg.padDualSampaChannel(padid),
  //             seg.padPositionX(padid), seg.padPositionY(padid));
  //
  std::vector<std::vector<o2::mch::Digit>> digitsPerEvent;
  digitsPerEvent.reserve(nofEvents); // one vector of digits per event

  for (auto i = 0; i < nofEvents; i++) {
    std::vector<o2::mch::Digit> digits;
    for (auto deid : deids) {
      int n = deid % 100;
      for (auto j = 0; j < n; j++) {
        int padid = j;
        double adc = ((j * 2) << 10 | (j * 2));
        double time = 987;
        digits.emplace_back(time, deid, padid, adc);
      }
    }
    digitsPerEvent.push_back(digits);
  }
  return digitsPerEvent;
}

std::vector<std::vector<o2::mch::Digit>> generateRandomDigits(int nofEvents, gsl::span<int> deids, float occupancy)
{
  std::vector<std::vector<o2::mch::Digit>> digitsPerEvent;
  digitsPerEvent.reserve(nofEvents); // one vector of digits per event
  // std::random_device rd;
  // std::mt19937 gen(rd());
  std::mt19937 gen(1234);

  std::vector<int> nofPads;
  int totalPads{0};
  for (auto d : deids) {
    auto npads = segmentation(d).nofPads();
    nofPads.emplace_back(npads);
    totalPads += npads;
  }

  std::poisson_distribution<> dis(static_cast<int>(occupancy * totalPads));

  for (auto i = 0; i < nofEvents; i++) {
    // draw n from mu
    int n = static_cast<int>(dis(gen));
    // generate n random digits
    auto digits = makeNRandomDigits(n, deids, gsl::make_span(nofPads));
    // add those n digits into this event
    digitsPerEvent.push_back(digits);
  }
  return digitsPerEvent;
}
