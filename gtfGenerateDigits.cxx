// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "gtfGetDigits.h"
#include "MCHMappingFactory/CreateSegmentation.h"

std::vector<std::vector<MCHDigit>> getDigits(int nofEvents, gsl::span<int> deids, float occupancy)
{
  std::vector<std::vector<MCHDigit>> digitsPerEvent;
  digitsPerEvent.reserve(nofEvents); // one vector of digits per event
  std::random_device rd;
  std::mt19937 gen(rd());

  std::vector<int> nofPads;
  int totalPads{0};
  for (auto d : deids) {
    auto npads = mapping::segmentation(d).nofPads();
    nofPads.emplace_back(npads);
    totalPads += npads;
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
