// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "DataPointGenerator.h"
#include <type_traits>
#include "DetectorsDCS/DataPointCompositeObject.h"
#include <random>
#include "DetectorsDCS/DataPointCreator.h"
#include <fmt/format.h>

namespace
{
/**
* Generate random data points, uniformly distributed between two values.
*
* @tparam T the type of data points to be generated. Must be an arithmetic type
*
* @param aliases the list of aliases to be generated
* @param minValue the minimum value of the values to be generated
* @param maxValue the maximum value of the values to be generated
* @param refDate the date to be associated with all data points 
*        in `%Y-%b-%d %H:%M:%S` format
*
* @returns a vector of DataPointCompositeObject objects
*/
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
} // namespace

namespace o2::dcs
{
std::vector<o2::dcs::DataPointCompositeObject>
  generateRandomFBI(const std::vector<DataPointHint>& dphints)
{
  return {};
}
std::vector<o2::dcs::DataPointCompositeObject> generateRandomDelta(const std::vector<DataPointHint>& dphints)
{
  return {};
}

std::vector<std::string> splitString(const std::string& src, char delim)
{
  std::stringstream ss(src);
  std::string token;
  std::vector<std::string> tokens;

  while (std::getline(ss, token, delim)) {
    if (!token.empty()) {
      tokens.push_back(std::move(token));
    }
  }

  return tokens;
}

std::vector<std::string> extractList(const std::string& slist)
{
  auto dots = slist.find(",");
  if (dots == std::string::npos) {
    return {};
  }
  return splitString(slist, ',');
}

std::vector<std::string> extractRange(const std::string& range)
{
  auto dots = range.find("..");
  if (dots == std::string::npos) {
    return extractList(range);
  }

  auto sa = range.substr(0, dots);
  auto sb = range.substr(dots + 2);
  auto size = std::max(sa.size(), sb.size());

  auto a = std::stoi(sa);

  auto b = std::stoi(sb);
  std::vector<std::string> result;

  for (auto i = a; i <= b; i++) {
    result.emplace_back(fmt::format("{0:0{1}d}", i, size));
  }
  return result;
}

std::vector<std::string> expandAlias(const std::string& pattern)
{
  auto leftBracket = pattern.find("[");
  auto rightBracket = pattern.find("]");

  // no bracket at all -> return pattern simply
  if (leftBracket == std::string::npos && rightBracket == std::string::npos) {
    return {pattern};
  }

  // no matching bracket -> wrong pattern -> return nothing
  if ((leftBracket == std::string::npos &&
       rightBracket != std::string::npos) ||
      (leftBracket != std::string::npos &&
       rightBracket == std::string::npos)) {
    return {};
  }
  auto rangeStr = pattern.substr(leftBracket + 1, rightBracket - leftBracket - 1);

  auto range = extractRange(rangeStr);

  // incorrect range -> return nothing
  if (range.empty()) {
    return {};
  }

  auto newPattern = pattern.substr(0, leftBracket) +
                    "{:s}" +
                    pattern.substr(rightBracket + 1);

  std::vector<std::string> result;

  for (auto r : range) {
    auto substituted = fmt::format(newPattern, r);
    result.emplace_back(substituted);
  }

  return expandAliases(result);
}

std::vector<std::string> expandAliases(const std::vector<std::string>& patternedAliases)
{
  std::vector<std::string> result;

  for (auto a : patternedAliases) {
    auto e = expandAlias(a);
    result.insert(result.end(), e.begin(), e.end());
  }
  // sort to get a predictable result
  std::sort(result.begin(), result.end());

  return result;
}

} // namespace o2::dcs
