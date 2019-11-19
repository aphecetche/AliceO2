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

// generate n digits randomly distributed over the detection elements
// whose ids are within the deids span
std::vector<o2::mch::Digit> generateRandomDigits(int n, gsl::span<int> deids, gsl::span<int> nofpads)
{
  std::vector<o2::mch::Digit> digits;
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

std::vector<std::vector<o2::mch::Digit>> generateDigits(int nofEvents, gsl::span<int> deids, float occupancy)
{
  std::vector<std::vector<o2::mch::Digit>> digitsPerEvent;
  digitsPerEvent.reserve(nofEvents); // one vector of digits per event
  std::random_device rd;
  std::mt19937 gen(rd());

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
    auto digits = generateRandomDigits(n, deids, gsl::make_span(nofPads));
    // add those n digits into this event
    digitsPerEvent.push_back(digits);
  }
  return digitsPerEvent;
}
