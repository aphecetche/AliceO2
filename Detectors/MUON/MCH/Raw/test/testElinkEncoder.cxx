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
#include "MCHRaw/ElinkEncoder.h"
#include "MCHRaw/SampaHeader.h"
#include <fstream>
#include <fmt/printf.h>
#include "common.h"

using namespace o2::mch::raw;

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(elinkencoder)

BOOST_AUTO_TEST_CASE(CtorMustUse4BitsOnlyForChipAddress)
{
  BOOST_CHECK_THROW(ElinkEncoder enc(0, 32), std::invalid_argument);
  BOOST_CHECK_THROW(ElinkEncoder enc(0, 16), std::invalid_argument);
  BOOST_CHECK_NO_THROW(ElinkEncoder enc(0, 15));
}

BOOST_AUTO_TEST_CASE(CtorMustHaveIdBetween0And39)
{
  BOOST_CHECK_THROW(ElinkEncoder enc(40, 0), std::invalid_argument);
  BOOST_CHECK_NO_THROW(ElinkEncoder enc(39, 0));
}

BOOST_AUTO_TEST_CASE(CtorBuildsAnEmptyBitSet)
{
  ElinkEncoder enc(0, 0);
  BOOST_CHECK_EQUAL(enc.len(), 0);
}

BOOST_AUTO_TEST_CASE(AddEmptyDataShouldThrow)
{
  ElinkEncoder enc(0, 0);
  std::vector<SampaCluster> data;
  BOOST_CHECK_THROW(enc.addChannelData(31, data), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(AddMixedDataShouldThrow)
{
  ElinkEncoder enc(0, 0);
  std::vector<SampaCluster> data;
  std::vector<uint16_t> samples{123, 456, 789};
  data.emplace_back(0, 1000);
  data.emplace_back(0, samples);
  BOOST_CHECK_THROW(enc.addChannelData(31, data), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(AddSingleHitShouldIncreaseSizeBy140Bits)
{
  ElinkEncoder enc(0, 0);
  auto initialSize = enc.len();
  std::vector<SampaCluster> data = {SampaCluster(20, 10)};
  enc.addChannelData(31, data);
  int expectedSize = initialSize + 100 + 40;
  BOOST_CHECK_EQUAL(enc.len(), expectedSize);
}

BOOST_AUTO_TEST_CASE(AddMultipleHitsShouldIncreateSizeBy140BitsTimeN)
{
  ElinkEncoder enc(0, 0);
  auto initialSize = enc.len();
  uint8_t chId{31};

  std::vector<SampaCluster> data = {
    SampaCluster(10, 1000),
    SampaCluster(20, 2000),
    SampaCluster(30, 3000),
  };

  enc.addChannelData(chId, data);

  int expectedSize = initialSize + 100 + 40 * data.size();
  BOOST_CHECK_EQUAL(enc.len(), expectedSize);
}

BOOST_AUTO_TEST_CASE(ChannelIdIs5Bits)
{
  ElinkEncoder enc(0, 0);
  std::vector<SampaCluster> data = {SampaCluster(20, 10)};

  BOOST_CHECK_THROW(enc.addChannelData(32, data),
                    std::invalid_argument);
  BOOST_CHECK_NO_THROW(enc.addChannelData(31, data));
}

BOOST_AUTO_TEST_CASE(OneChipChargeSumOneCluster)
{
  ElinkEncoder enc(0, 9, 20);
  auto initialSize = enc.len();
  enc.addChannelData(1, {SampaCluster(20, 101)});
  enc.addChannelData(5, {SampaCluster(100, 505)});
  enc.addChannelData(13, {SampaCluster(260, 1313)});
  enc.addChannelData(31, {SampaCluster(620, 3131)});
  BOOST_CHECK_EQUAL(enc.len(), initialSize + 50 + 4 * 90);
}

BOOST_AUTO_TEST_CASE(OneChipSamplesOneCluster)
{
  uint8_t cruId{0};
  uint8_t linkId{0};
  int phase{0};
  bool clusterSumMode{false};
  ElinkEncoder enc(cruId, linkId, phase, clusterSumMode);
  auto initialSize = enc.len();
  enc.addChannelData(1, {SampaCluster(20, std::vector<uint16_t>{1, 10, 100, 10, 1})});
  enc.addChannelData(5, {SampaCluster(100, std::vector<uint16_t>{5, 50, 5})});
  enc.addChannelData(13, {SampaCluster(260, std::vector<uint16_t>{
                                              13, 14, 15, 15, 13})});
  enc.addChannelData(31, {SampaCluster(620, std::vector<uint16_t>{31})});
  BOOST_CHECK_EQUAL(enc.len(), initialSize + 50 + 4 * (50 + 20) + 14 * 10);
}

BOOST_AUTO_TEST_CASE(GetShouldThrowIfBitNumberIsBeyondLen)
{
  ElinkEncoder enc = o2::mch::raw::test::createElinkEncoder20();

  BOOST_CHECK_THROW(enc.get(enc.len()), std::invalid_argument);
  BOOST_CHECK_NO_THROW(enc.get(enc.len() - 1));
}

BOOST_AUTO_TEST_CASE(FillWithSync)
{
  ElinkEncoder enc = o2::mch::raw::test::createElinkEncoder20();
  auto s = enc.len();
  enc.fillWithSync(s + 154);
  BOOST_CHECK_EQUAL(enc.len(), s + 154);
  BOOST_CHECK_EQUAL(enc.range(s, s + 49), sampaSync().uint64());
  BOOST_CHECK_EQUAL(enc.range(s + 50, s + 99), sampaSync().uint64());
  BOOST_CHECK_EQUAL(enc.range(s + 100, s + 149), sampaSync().uint64());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
