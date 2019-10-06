// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#define BOOST_TEST_MODULE Test MCHRaw GBTEncoder
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include "MCHRaw/GBTEncoder.h"
#include <array>
#include "common.h"
#include <array>

using namespace o2::mch::raw;

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(gbtencoder)

BOOST_AUTO_TEST_CASE(EncodeABuffer)
{
  auto buffer = getEncodedBuffer();
  BOOST_CHECK_GE(buffer.size(), REF_BUFFER.size());
  BOOST_CHECK(std::equal(begin(buffer), end(buffer), begin(REF_BUFFER)));
}

BOOST_AUTO_TEST_CASE(GBTEncoderLinkIdMustBeBetween0And23)
{
  BOOST_CHECK_THROW(GBTEncoder enc(0, 24), std::invalid_argument);
  BOOST_CHECK_NO_THROW(GBTEncoder enc(0, 23));
}

BOOST_AUTO_TEST_CASE(GBTEncoderAddFewChannels)
{
  GBTEncoder enc(0, 0);
  uint32_t bx(0);
  uint16_t ts(0);
  int elinkId = 0;
  enc.addChannelChargeSum(elinkId, ts, 0, 10);
  enc.addChannelChargeSum(elinkId, ts, 31, 160);
  elinkId = 3;
  enc.addChannelChargeSum(elinkId, ts, 3, 13);
  enc.addChannelChargeSum(elinkId, ts, 13, 133);
  enc.addChannelChargeSum(elinkId, ts, 23, 163);
  BOOST_CHECK_THROW(enc.addChannelChargeSum(40, ts, 0, 10), std::invalid_argument);
  enc.printStatus(4);
  std::vector<uint32_t> buffer;
  enc.moveToBuffer(buffer);
  // we only test >= because the exact size can vary
  // due to the initial phase used in the elinks
  BOOST_CHECK_GE(buffer.size(), 640);
}

BOOST_AUTO_TEST_CASE(GBTEncoderAdd64Channels)
{
  std::vector<uint32_t> buffer;
  GBTEncoder enc(0, 0);
  enc.moveToBuffer(buffer);
  std::cout << buffer.size() << "\n";
  uint32_t bx(0);
  uint16_t ts(0);
  int elinkId = 0;
  for (int i = 0; i < 64; i++) {
    enc.addChannelChargeSum(elinkId, ts, i % 32, i * 10);
  }
  enc.moveToBuffer(buffer);
  BOOST_CHECK_GE(buffer.size(), 11620); // nof gbt words
}

BOOST_AUTO_TEST_CASE(GBTEncoderMoveToBufferClearsTheInternalBuffer)
{
  GBTEncoder enc(0, 0);
  enc.addChannelChargeSum(0, 0, 0, 10);
  std::vector<uint32_t> buffer;
  size_t n = enc.moveToBuffer(buffer);
  BOOST_CHECK_GE(n, 280);
  n = enc.moveToBuffer(buffer);
  BOOST_CHECK_EQUAL(n, 0);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
