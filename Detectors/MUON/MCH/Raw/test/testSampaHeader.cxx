// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

///
/// @author  Laurent Aphecetche

#define BOOST_TEST_MODULE Test MCHRaw SampaHeader
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <iostream>
#include "MCHRaw/SampaHeader.h"
#include <fstream>
#include <fmt/printf.h>

using namespace o2::mch::raw;

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(sampaheader)

BOOST_AUTO_TEST_CASE(SampaHeaderEqualityOperators)
{
  // comparison is full comparison (i.e. equality of 50bits)

  SampaHeader h(sampaSync());

  BOOST_CHECK(h == sampaSync());

  SampaHeader h2(UINT64_C(0x1fffff5f0007f));

  BOOST_CHECK(h2 != sampaSync());
}

BOOST_AUTO_TEST_CASE(SampaHeaderSetHamming)
{
  SampaHeader sh;

  BOOST_CHECK_THROW(sh.hammingCode(1 << 6), std::invalid_argument);
  sh.hammingCode(0x3F);
  BOOST_CHECK_EQUAL(sh.hammingCode(), 0X3F);
}

BOOST_AUTO_TEST_CASE(SampaHeaderSetHeaderParity)
{
  SampaHeader sh;

  sh.headerParity(true);
  BOOST_CHECK_EQUAL(sh.headerParity(), true);
}

BOOST_AUTO_TEST_CASE(SampaHeaderSetPacketType)
{
  SampaHeader sh;

  BOOST_CHECK_THROW(sh.packetType(1 << 3), std::invalid_argument);
  sh.packetType(0x7);
  BOOST_CHECK_EQUAL(sh.packetType(), 7);
}

BOOST_AUTO_TEST_CASE(SampaHeaderSetNumberOf10BitsWords)
{
  SampaHeader sh;

  BOOST_CHECK_THROW(sh.nbOf10BitWords(1 << 10), std::invalid_argument);
  sh.nbOf10BitWords(0x3FF);
  BOOST_CHECK_EQUAL(sh.nbOf10BitWords(), 0x3FF);
}

BOOST_AUTO_TEST_CASE(SampaHeaderSetChipAddress)
{
  SampaHeader sh;

  BOOST_CHECK_THROW(sh.chipAddress(1 << 4), std::invalid_argument);
  sh.chipAddress(0xF);
  BOOST_CHECK_EQUAL(sh.chipAddress(), 0xF);
}

BOOST_AUTO_TEST_CASE(SampaHeaderSetChannelAddress)
{
  SampaHeader sh;

  BOOST_CHECK_THROW(sh.channelAddress(1 << 5), std::invalid_argument);
  sh.channelAddress(0x1F);
  BOOST_CHECK_EQUAL(sh.channelAddress(), 0x1F);
}

BOOST_AUTO_TEST_CASE(SampaHeaderSetBunchCrossingCounter)
{
  SampaHeader sh;

  BOOST_CHECK_THROW(sh.bunchCrossingCounter(1 << 20), std::invalid_argument);
  sh.bunchCrossingCounter(0xFFFFF);
  BOOST_CHECK_EQUAL(sh.bunchCrossingCounter(), 0xFFFFF);
}

BOOST_AUTO_TEST_CASE(SampaHeaderSetPayloadParity)
{
  SampaHeader sh;

  sh.payloadParity(true);
  BOOST_CHECK_EQUAL(sh.payloadParity(), true);
}

BOOST_AUTO_TEST_CASE(SampaHeaderLessThanOperatorComparesBx)
{
  SampaHeader h1;
  SampaHeader h2;

  h1.bunchCrossingCounter(1);
  h2.bunchCrossingCounter(2);

  SampaHeader h10{h1};

  BOOST_CHECK(h1 == h10);

  BOOST_CHECK_EQUAL(h1 > h2, false);
  BOOST_CHECK_EQUAL(h1 < h2, true);
  BOOST_CHECK_EQUAL(h1 <= h10, true);
  BOOST_CHECK_EQUAL(h1 >= h10, true);
}

BOOST_AUTO_TEST_CASE(SampaHeaderCtorWithMoreThan50BitsShouldThrow)
{
  BOOST_CHECK_THROW(SampaHeader(static_cast<uint64_t>(1) << 50), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(SampaHeaderCtorWithInvalidBitsIsNotAHeartbeat)
{
  uint64_t h = 0x3FFFFEAFFFFFF; // completely invalid value to start with
  uint64_t one = 1;

  // - bits 7-9 must be zero
  // - bits 10-19 must be zero
  // - bits 24,26,28 must be one
  // - bits 25,27 must be zero
  // - bit 49 must be zero

  std::vector<int> zeros = {7, 8, 9, 24, 26, 28, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 25, 27, 49};
  std::vector<int> ones = {24, 26, 28};

  BOOST_CHECK_EQUAL(SampaHeader(h).isHeartbeat(), false);
  for (auto ix : zeros) {
    h &= ~(one << ix);
    BOOST_CHECK_EQUAL(SampaHeader(h).isHeartbeat(), false);
  }
  for (auto ix : ones) {
    h |= (one << ix);
  }
  BOOST_CHECK_EQUAL(SampaHeader(h).isHeartbeat(), true);
}

BOOST_AUTO_TEST_CASE(SampaHeaderCtorWithIncorrectNumberOfPartialBitsShouldThrow)
{
  // hamming is 6 bits max
  // pkt is 3 bits max
  // numWords is 10 bits max
  // h is 4 bits max
  // ch is 5 bits max
  // bx is 20 bits max
  BOOST_CHECK_THROW(SampaHeader(1 << 6, true, 0, 0, 0, 0, 0, true),
                    std::invalid_argument);
  BOOST_CHECK_THROW(SampaHeader(0, true, 1 << 3, 0, 0, 0, 0, true),
                    std::invalid_argument);
  BOOST_CHECK_THROW(SampaHeader(0, true, 0, 1 << 10, 0, 0, 0, true),
                    std::invalid_argument);
  BOOST_CHECK_THROW(SampaHeader(0, true, 0, 0, 1 << 4, 0, 0, true),
                    std::invalid_argument);
  BOOST_CHECK_THROW(SampaHeader(0, true, 0, 0, 0, 1 << 5, 0, true),
                    std::invalid_argument);
  BOOST_CHECK_THROW(SampaHeader(0, true, 0, 0, 0, 0, 1 << 20, true),
                    std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(CheckSampaSyncIsExpectedValue)
{
  SampaHeader h(0x1555540f00113);
  BOOST_CHECK(sampaSync() == h);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
