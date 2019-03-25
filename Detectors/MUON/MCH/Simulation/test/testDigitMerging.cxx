// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>

#include "MCHSimulation/Digit.h"
#include "DigitMerging.h"
#include "boost/format.hpp"
#include <boost/test/data/test_case.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>

using o2::mch::Digit;

BOOST_AUTO_TEST_SUITE(o2_mch_simulation)

std::vector<Digit> createNonOverlappingDigits()
{
  return std::vector<Digit>{
    { 2, 5 },
    { 3, 6 },
    { 1, 2 },
    { 0, 1 }
  };
}
std::vector<Digit> createOverlappingDigits()
{
  return std::vector<Digit>{
    { 2, 5 },
    { 3, 6 },
    { 1, 2 },
    { 0, 0 },
    { 0, 1 },
    { 1, 3 },
    { 3, 7 },
    { 1, 4 }
  };
}

std::vector<Digit> expected()
{
  return std::vector<Digit>{
    { 0, 1 },
    { 1, 9 },
    { 2, 5 },
    { 3, 13 }
  };
}

BOOST_DATA_TEST_CASE(DigitMergingIdentity, boost::unit_test::data::make(mergingFunctions()), mergingFunction)
{
  auto m = mergingFunction(createNonOverlappingDigits());
  auto e = m;
  BOOST_CHECK(std::is_permutation(m.begin(), m.end(), e.begin()));
}

BOOST_DATA_TEST_CASE(DigitMerging, boost::unit_test::data::make(mergingFunctions()), mergingFunction)
{
  auto m = mergingFunction(createOverlappingDigits());
  BOOST_CHECK(std::is_permutation(m.begin(), m.end(), expected().begin()));
}

BOOST_AUTO_TEST_SUITE_END()
