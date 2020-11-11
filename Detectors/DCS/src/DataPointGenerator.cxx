// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "DetectorsDCS/AliasExpander.h"
#include "DetectorsDCS/DataPointGenerator.h"
#include "DetectorsDCS/DataPointCreator.h"
#include "DetectorsDCS/DataPointCompositeObject.h"
#include <fmt/format.h>
#include <random>

namespace
{
o2::dcs::DataPointCompositeObject createDPCOM(const std::string& alias, const uint64_t* val, uint32_t seconds, uint16_t msec, uint16_t flags, o2::dcs::DeliveryType dt)
{
  auto dpid = o2::dcs::DataPointIdentifier(alias, dt);
  auto dpval = o2::dcs::DataPointValue(
    flags,
    msec,
    seconds,
    val,
    dt);
  return o2::dcs::DataPointCompositeObject(dpid, dpval);
}

} // namespace

namespace o2::dcs
{

/**
* Generate random data points, uniformly distributed between two values.
*
* @param aliases the list of aliases to be generated
* @param dt ADAPOS delivery type. Only the types that are directly
*   convertible to uint64_t are supported.
* @param minValue the minimum value of the values to be generated
* @param maxValue the maximum value of the values to be generated
* @param refDate the date to be associated with all data points 
*        in `%Y-%b-%d %H:%M:%S` format
*
* @returns a vector of DataPointCompositeObject objects
*/
template <typename T>
std::vector<o2::dcs::DataPointCompositeObject> generateRandomDataPoints(const std::vector<std::string>& aliases,
                                                                        T minValue, T maxValue, const std::string& refDate)
{
  static_assert(std::is_arithmetic<T>::value, "T must be an arithmetic type");
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
  for (auto alias : expandAliases(aliases)) {
    auto value = dist(mt);
    dpcoms.emplace_back(o2::dcs::createDataPointCompositeObject(alias, value, seconds, msec));
  }
  return dpcoms;
}

// only specialize the functions for the types we support :
//
// - double
// - uint32_t
// - int32_t
// - char 
// - bool

template std::vector<o2::dcs::DataPointCompositeObject> generateRandomDataPoints<double>(const std::vector<std::string>& aliases, double minValue, double maxValue, const std::string&);

template std::vector<o2::dcs::DataPointCompositeObject> generateRandomDataPoints<uint32_t>(const std::vector<std::string>& aliases, uint32_t minValue, uint32_t maxValue, const std::string&);

template std::vector<o2::dcs::DataPointCompositeObject> generateRandomDataPoints<int32_t>(const std::vector<std::string>& aliases, int32_t minValue, int32_t maxValue, const std::string&);

template std::vector<o2::dcs::DataPointCompositeObject> generateRandomDataPoints<bool>(const std::vector<std::string>& aliases, bool minValue, bool maxValue, const std::string&);

template std::vector<o2::dcs::DataPointCompositeObject> generateRandomDataPoints<char>(const std::vector<std::string>& aliases, char minValue, char maxValue, const std::string&);
} // namespace o2::dcs
