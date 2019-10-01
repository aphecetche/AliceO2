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

BOOST_AUTO_TEST_CASE(ElinkEncoderCtorMustUse4BitsOnlyForDsId)
{
  BOOST_CHECK_THROW(ElinkEncoder enc(0, 32), std::invalid_argument);
  BOOST_CHECK_THROW(ElinkEncoder enc(0, 16), std::invalid_argument);
  BOOST_CHECK_NO_THROW(ElinkEncoder enc(0, 15));
}

BOOST_AUTO_TEST_CASE(ElinkEncoderIdMustBeBetween0And39)
{
  BOOST_CHECK_THROW(ElinkEncoder enc(40, 0), std::invalid_argument);
  BOOST_CHECK_NO_THROW(ElinkEncoder enc(39, 0));
}

BOOST_AUTO_TEST_CASE(BunchCrossingCounterMustBeWithin20Bits)
{
  ElinkEncoder enc(0, 0);

  BOOST_CHECK_THROW(enc.bunchCrossingCounter(0x1FFFFF), std::invalid_argument);
  enc.bunchCrossingCounter(0xFFFFF);
  BOOST_CHECK_NO_THROW(enc.bunchCrossingCounter(0xFFFFF));
}

BOOST_AUTO_TEST_CASE(ElinkEncoderCtorBuildsAnEmptyBitSet)
{
  ElinkEncoder enc(0, 0);
  BOOST_CHECK_EQUAL(enc.len(), 0);
}

BOOST_AUTO_TEST_CASE(ElinkEncoderAddRandomBits)
{
  ElinkEncoder enc(0, 0);
  enc.addRandomBits(13);
  BOOST_CHECK_EQUAL(enc.len(), 13);
}

BOOST_AUTO_TEST_CASE(ElinkEncoderAddSync)
{
  ElinkEncoder enc(0, 0);
  enc.addSync();
  BOOST_CHECK_EQUAL(enc.len(), 50);
}

BOOST_AUTO_TEST_CASE(EncodeChannelId5Bits)
{
  ElinkEncoder enc(0, 0);

  BOOST_CHECK_THROW(enc.addChannelChargeSum(32, 20, 10),
                    std::invalid_argument);
  BOOST_CHECK_NO_THROW(enc.addChannelChargeSum(31, 20, 10));
  BOOST_CHECK_THROW(enc.addChannelSamples(32, 20, {10}),
                    std::invalid_argument);
  BOOST_CHECK_NO_THROW(enc.addChannelSamples(31, 20, {10}));
}

BOOST_AUTO_TEST_CASE(EncodeTimeStamp10Bits)
{
  ElinkEncoder enc(0, 0);

  BOOST_CHECK_THROW(enc.addChannelChargeSum(31, 0x7FF, 10),
                    std::invalid_argument);
  BOOST_CHECK_NO_THROW(enc.addChannelChargeSum(31, 0x3FF, 10));
}

BOOST_AUTO_TEST_CASE(EncodeSamples10BitsIfSeveralSamples)
{
  ElinkEncoder enc(0, 0);

  BOOST_CHECK_THROW(enc.addChannelSamples(31, 0, {0x7FF, 0x1FF}),
                    std::invalid_argument);
  BOOST_CHECK_NO_THROW(enc.addChannelSamples(31, 0, {0x1FF, 0x1FF}));
}

BOOST_AUTO_TEST_CASE(EncodeSamples20BitsIfOnlyOneSample)
{
  ElinkEncoder enc(0, 0);

  BOOST_CHECK_THROW(enc.addChannelChargeSum(31, 0, 0x1FFFFF),
                    std::invalid_argument);
  BOOST_CHECK_NO_THROW(enc.addChannelChargeSum(31, 0, 0xFFFFF));
}

BOOST_AUTO_TEST_CASE(EncodeOneDSChargeSum)
{
  ElinkEncoder enc = encoderExample1();
  BOOST_CHECK_EQUAL(enc.len(), 13 + 50 + 4 * 90);
}

BOOST_AUTO_TEST_CASE(EncodeOneDSSamples)
{
  ElinkEncoder enc(0, 9);

  enc.addChannelSamples(1, 20, {1, 10, 100, 10, 1});
  enc.addChannelSamples(5, 100, {5, 50, 5});
  enc.addChannelSamples(13, 260, {13, 14, 15, 15, 13});
  enc.addChannelSamples(31, 620, {31});

  BOOST_CHECK_EQUAL(enc.len(), 4 * (50 + 20) + 14 * 10);
}

BOOST_AUTO_TEST_CASE(EncoderGetShouldThrowIfBitNumberIsBeyondLen)
{
  ElinkEncoder enc = encoderExample1();

  BOOST_CHECK_THROW(enc.get(enc.len()), std::invalid_argument);
  BOOST_CHECK_NO_THROW(enc.get(enc.len() - 1));
}

BOOST_AUTO_TEST_CASE(EncoderFillWithSync)
{
  ElinkEncoder enc = encoderExample1();
  auto s = enc.len();
  enc.fillWithSync(s + 154);
  BOOST_CHECK_EQUAL(enc.len(), s + 154);
  BOOST_CHECK_EQUAL(enc.range(s, s + 49), sampaSync().uint64());
  BOOST_CHECK_EQUAL(enc.range(s + 50, s + 99), sampaSync().uint64());
  BOOST_CHECK_EQUAL(enc.range(s + 100, s + 149), sampaSync().uint64());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
