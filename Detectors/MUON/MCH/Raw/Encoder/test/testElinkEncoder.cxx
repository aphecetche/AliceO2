// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#define BOOST_TEST_MODULE Test MCHRaw ElinkEncoder
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <iostream>
#include "BareElinkEncoder.h"
#include "UserLogicElinkEncoder.h"
#include "MCHRawCommon/SampaHeader.h"
#include <fstream>
#include <fmt/printf.h>
#include <boost/mpl/list.hpp>

using namespace o2::mch::raw;

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(elinkencoder)

typedef boost::mpl::list<BareElinkEncoder, UserLogicElinkEncoder> testTypes;

BOOST_AUTO_TEST_CASE_TEMPLATE(CtorMustUse4BitsOnlyForChipAddress, T, testTypes)
{
  BOOST_CHECK_THROW(T enc(0, 32), std::invalid_argument);
  BOOST_CHECK_THROW(T enc(0, 16), std::invalid_argument);
  BOOST_CHECK_NO_THROW(T enc(0, 15));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(CtorMustHaveIdBetween0And39, T, testTypes)
{
  BOOST_CHECK_THROW(T enc(40, 0), std::invalid_argument);
  BOOST_CHECK_NO_THROW(T enc(39, 0));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(AddEmptyDataShouldThrow, T, testTypes)
{
  T enc(0, 0);
  std::vector<SampaCluster> data;
  BOOST_CHECK_THROW(enc.addChannelData(31, data), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(AddMixedDataShouldThrow, T, testTypes)
{
  T enc(0, 0);
  std::vector<SampaCluster> data;
  std::vector<uint16_t> samples{123, 456, 789};
  data.emplace_back(0, 1000);
  data.emplace_back(0, samples);
  BOOST_CHECK_THROW(enc.addChannelData(31, data), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ChannelIdIs5Bits, T, testTypes)
{
  T enc(0, 0);
  std::vector<SampaCluster> data = {SampaCluster(20, 10)};

  BOOST_CHECK_THROW(enc.addChannelData(32, data),
                    std::invalid_argument);
  BOOST_CHECK_NO_THROW(enc.addChannelData(31, data));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
