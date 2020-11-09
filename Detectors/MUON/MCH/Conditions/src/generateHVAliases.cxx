// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "DetectorsDCS/DataPointCreator.h"
#include "MCHConditions/DCSNamer.h"
#include <iostream>
#include <random>
#include "DetectorsDCS/GenericFunctions.h"
#include <ctime>
#include <sstream>
#include <locale>
#include <type_traits>

using namespace o2::mch::dcs;

//DPVAL valchar(flags, milliseconds + tfid * 10, seconds + tfid, payload, mtypechar);

template <typename T,
          typename = std::enable_if_t<std::is_arithmetic<T>::value>>
std::vector<o2::dcs::DataPointCompositeObject> generateRandomDPCOM(const std::vector<std::string>& aliases,
                                                                   T minValue, T maxValue, const std::string& refDate)
{
  std::vector<o2::dcs::DataPointCompositeObject> dpcoms;
  typedef typename std::conditional<std::is_integral<T>::value,
                                    std::uniform_int_distribution<T>,
                                    std::uniform_real_distribution<T>>::type distType;

  std::random_device rd;
  std::mt19937 mt(rd());
  distType dist{minValue, maxValue};
  std::tm t = {};
  std::istringstream ss(refDate);
  ss >> std::get_time(&t, "%Y-%b-%d %H:%M:%S");
  uint32_t seconds = mktime(&t);
  uint16_t msec = 5;
  for (auto alias : aliases) {
    auto value = dist(mt);
    dpcoms.emplace_back(o2::dcs::createDataPointCompositeObject(alias, value, seconds, msec));
  }
  return dpcoms;
}

int main()
{
  auto voltages = generateRandomDPCOM(aliases({MeasurementType::Voltage}), 1400.0, 1700.0, "2013-November-18 12:34:56");

  int nmax{5};
  int n{0};
  for (auto v : voltages) {
    n++;
    if (n > nmax) {
      break;
    }
    std::cout << v << "\n";
  }

  auto currents = generateRandomDPCOM<int32_t>(aliases({MeasurementType::Current}), 0, 20, "2019-November-18 12:34:56");

  n = 0;
  for (auto i : currents) {
    n++;
    if (n > nmax) {
      break;
    }
    std::cout << i << "\n";
  }

  auto C2 = generateRandomDPCOM(aliases({MeasurementType::Current}), 'A', 'z',
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
