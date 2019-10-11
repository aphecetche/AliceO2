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

BOOST_AUTO_TEST_CASE(AddSingleHitWithInvalidTimeStampShouldThrow)
{
  ElinkEncoder enc(0, 0);
  BOOST_CHECK_THROW(enc.addChannelChargeSum(31, 1 << 10, 0), std::invalid_argument);
  BOOST_CHECK_NO_THROW(enc.addChannelChargeSum(31, 0x3FF, 0));
}

BOOST_AUTO_TEST_CASE(AddSingleHitWithInvalidChargeSumShouldThrow)
{
  ElinkEncoder enc(0, 0);
  BOOST_CHECK_THROW(enc.addChannelChargeSum(31, 0, 1 << 20), std::invalid_argument);
  BOOST_CHECK_NO_THROW(enc.addChannelChargeSum(31, 0, 0xFFFFF));
}

BOOST_AUTO_TEST_CASE(AddSingleHitShouldIncreaseSizeBy140Bits)
{
  ElinkEncoder enc(0, 0);
  auto initialSize = enc.len();
  enc.addChannelChargeSum(31, 20, 10);
  int expectedSize = initialSize + 100 + 40;
  BOOST_CHECK_EQUAL(enc.len(), expectedSize);
}

BOOST_AUTO_TEST_CASE(AddMultipleHitsWithInvalidDataShouldThrow)
{
  ElinkEncoder enc(0, 0);
  auto initialSize = enc.len();
  uint8_t chId{31};
  std::vector<uint16_t> nsamples = {3, 1, 1};
  std::vector<uint16_t> timestamp = {10, 20, 30};
  std::vector<uint32_t> chargeSum = {1000, 2000, 3000};
  std::vector<uint16_t> invalidNsamples = {3, 1 << 10, 1};
  std::vector<uint16_t> invalidTimestamp = {1 << 10, 20, 30};
  std::vector<uint32_t> invalidChargeSum = {1000, 2000, 1 << 20};

  BOOST_CHECK_NO_THROW(enc.addChannelChargeSum(chId, nsamples, timestamp, chargeSum));
  BOOST_CHECK_THROW(enc.addChannelChargeSum(chId, invalidNsamples, timestamp, chargeSum), std::invalid_argument);
  BOOST_CHECK_THROW(enc.addChannelChargeSum(chId, nsamples, invalidTimestamp, chargeSum), std::invalid_argument);
  BOOST_CHECK_THROW(enc.addChannelChargeSum(chId, nsamples, timestamp, invalidChargeSum), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(AddMultipleHitsShouldIncreateSizeBy140BitsTimeN)
{
  ElinkEncoder enc(0, 0);
  auto initialSize = enc.len();
  uint8_t chId{31};
  std::vector<uint16_t> nsamples = {3, 1, 1};
  std::vector<uint16_t> timestamp = {10, 20, 30};
  std::vector<uint32_t> chargeSum = {1000, 2000, 3000};

  enc.addChannelChargeSum(chId, nsamples, timestamp, chargeSum);

  int expectedSize = initialSize + 100 + 40 * nsamples.size();
  BOOST_CHECK_EQUAL(enc.len(), expectedSize);
}

BOOST_AUTO_TEST_CASE(ChannelIdIs5Bits)
{
  ElinkEncoder enc(0, 0);

  BOOST_CHECK_THROW(enc.addChannelChargeSum(32, 20, 10),
                    std::invalid_argument);
  BOOST_CHECK_NO_THROW(enc.addChannelChargeSum(31, 20, 10));
  BOOST_CHECK_THROW(enc.addChannelSamples(32, 20, {10}),
                    std::invalid_argument);
  BOOST_CHECK_NO_THROW(enc.addChannelSamples(31, 20, {10}));
}

BOOST_AUTO_TEST_CASE(TimeStampIs10Bits)
{
  ElinkEncoder enc(0, 0);

  BOOST_CHECK_THROW(enc.addChannelChargeSum(31, 0x7FF, 10),
                    std::invalid_argument);
  BOOST_CHECK_NO_THROW(enc.addChannelChargeSum(31, 0x3FF, 10));
}

BOOST_AUTO_TEST_CASE(SamplesAre10Bits)
{
  ElinkEncoder enc(0, 0);

  BOOST_CHECK_THROW(enc.addChannelSamples(31, 0, {0x7FF, 0x1FF}),
                    std::invalid_argument);
  BOOST_CHECK_NO_THROW(enc.addChannelSamples(31, 0, {0x1FF, 0x1FF}));
}

BOOST_AUTO_TEST_CASE(SamplesAre20BitsIfOnlyOneSample)
{
  ElinkEncoder enc(0, 0);

  BOOST_CHECK_THROW(enc.addChannelChargeSum(31, 0, 0x1FFFFF),
                    std::invalid_argument);
  BOOST_CHECK_NO_THROW(enc.addChannelChargeSum(31, 0, 0xFFFFF));
}

BOOST_AUTO_TEST_CASE(OneChipChargeSumOneCluster)
{
  ElinkEncoder enc(0, 9, 20);
  auto initialSize = enc.len();
  enc.addChannelChargeSum(1, 20, 101);
  enc.addChannelChargeSum(5, 100, 505);
  enc.addChannelChargeSum(13, 260, 1313);
  enc.addChannelChargeSum(31, 620, 3131);
  BOOST_CHECK_EQUAL(enc.len(), initialSize + 50 + 4 * 90);
}

BOOST_AUTO_TEST_CASE(OneChipSamplesOneCluster)
{
  ElinkEncoder enc(0, 9);
  auto initialSize = enc.len();
  enc.addChannelSamples(1, 20, {1, 10, 100, 10, 1});
  enc.addChannelSamples(5, 100, {5, 50, 5});
  enc.addChannelSamples(13, 260, {13, 14, 15, 15, 13});
  enc.addChannelSamples(31, 620, {31});
  BOOST_CHECK_EQUAL(enc.len(), initialSize + 50 + 4 * (50 + 20) + 14 * 10);
}

BOOST_AUTO_TEST_CASE(GetShouldThrowIfBitNumberIsBeyondLen)
{
  ElinkEncoder enc = o2::mch::raw::test::createElinkEncoder();

  BOOST_CHECK_THROW(enc.get(enc.len()), std::invalid_argument);
  BOOST_CHECK_NO_THROW(enc.get(enc.len() - 1));
}

BOOST_AUTO_TEST_CASE(FillWithSync)
{
  ElinkEncoder enc = o2::mch::raw::test::createElinkEncoder();
  auto s = enc.len();
  enc.fillWithSync(s + 154);
  BOOST_CHECK_EQUAL(enc.len(), s + 154);
  BOOST_CHECK_EQUAL(enc.range(s, s + 49), sampaSync().uint64());
  BOOST_CHECK_EQUAL(enc.range(s + 50, s + 99), sampaSync().uint64());
  BOOST_CHECK_EQUAL(enc.range(s + 100, s + 149), sampaSync().uint64());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
