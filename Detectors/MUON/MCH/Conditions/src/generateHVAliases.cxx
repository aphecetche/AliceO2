// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHConditions/DCSNamer.h"
#include <iostream>
#include <random>
#include "DetectorsDCS/GenericFunctions.h"
#include <ctime>
#include <sstream>
#include <locale>
#include <type_traits>
#include "DetectorsDCS/DataPointGenerator.h"

using namespace o2::mch::dcs;

int main()
{
  auto voltages = o2::dcs::generateRandomDataPoints(aliases({MeasurementType::Voltage}), 1400.0, 1700.0, "2013-November-18 12:34:56");

  int nmax{100000};
  int n{0};
  for (auto v : voltages) {
    n++;
    if (n > nmax) {
      break;
    }
    std::cout << v << "\n";
  }

  auto currents = o2::dcs::generateRandomDataPoints<int32_t>(aliases({MeasurementType::Current}), 0, 20, "2019-November-18 12:34:56");

  n = 0;
  for (auto i : currents) {
    n++;
    if (n > nmax) {
      break;
    }
    std::cout << i << "\n";
  }

  auto C2 = o2::dcs::generateRandomDataPoints<char>(aliases({MeasurementType::Current}), 'A', 'z',
                                                    "2022-November-18 12:34:56");

  n = 0;
  for (auto i : C2) {
    n++;
    if (n > nmax) {
      break;
    }
    std::cout << i << "\n";
  }
  return 0;
}
