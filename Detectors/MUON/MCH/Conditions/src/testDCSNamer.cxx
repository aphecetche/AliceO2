// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#define BOOST_TEST_MODULE Test MCHConditions DCSNamer
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include "MCHConditions/DCSNamer.h"
#include "HVAliases.h"
#include "LVAliases.h"
#include <algorithm>
#include <fmt/printf.h>

using namespace o2::mch::dcs;

BOOST_AUTO_TEST_SUITE(o2_mch_conditions)

BOOST_AUTO_TEST_SUITE(dcsnamer)

BOOST_AUTO_TEST_CASE(detElemId2DCSMustReturnNothingIfDetElemIdIsNotValid)
{
  BOOST_CHECK_EQUAL(detElemId2DCS(0).has_value(), false);
  BOOST_CHECK_EQUAL(detElemId2DCS(10).has_value(), false);
  BOOST_CHECK_EQUAL(detElemId2DCS(1026).has_value(), false);
  BOOST_CHECK_EQUAL(detElemId2DCS(-1).has_value(), false);
}

BOOST_AUTO_TEST_CASE(DE100isRight)
{
  BOOST_CHECK(detElemId2DCS(100)->side == Side::Right);
}
BOOST_AUTO_TEST_CASE(DE102isLeft)
{
  BOOST_CHECK(detElemId2DCS(102)->side == Side::Left);
}

BOOST_AUTO_TEST_CASE(detElemId2DCSMustReturnChamberIdAndSideIfDetElemIdIsValid)
{
  auto v = detElemId2DCS(1025);
  BOOST_CHECK_EQUAL(v.has_value(), true);
  BOOST_CHECK_EQUAL(v->chamberId, 9);
  BOOST_CHECK(v->side == Side::Right);
}

BOOST_AUTO_TEST_CASE(NumberOfHVAliasesIs188PerType)
{
  auto result = aliases({MeasurementType::Voltage});
  for (auto r: result) {
      fmt::printf("%-62s %2d\n",r.c_str(),r.size());
  }
  BOOST_CHECK_EQUAL(result.size(), expectedHVAliasesVoltages.size());
  bool permutation = std::is_permutation(begin(result), end(result), begin(expectedHVAliasesVoltages));
  BOOST_CHECK_EQUAL(permutation, true);
  result = aliases({MeasurementType::Current});
  BOOST_CHECK_EQUAL(result.size(), expectedHVAliasesCurrents.size());
  permutation = std::is_permutation(begin(result), end(result), begin(expectedHVAliasesCurrents));
  BOOST_CHECK_EQUAL(permutation, true);
}

BOOST_AUTO_TEST_CASE(NumberOfLVAliasesPerType)
{
  auto result = aliases({MeasurementType::Analog, MeasurementType::Digital});
  BOOST_CHECK_EQUAL(result.size(), expectedLVAliases.size());
  bool permutation = std::is_permutation(begin(result), end(result), begin(expectedLVAliases));
  BOOST_CHECK_EQUAL(permutation, true);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
