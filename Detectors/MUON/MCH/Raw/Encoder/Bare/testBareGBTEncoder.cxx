// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#define BOOST_TEST_MODULE Test MCHRaw BareGBTEncoder
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include "BareGBTEncoder.h"
#include <array>
#include <array>
#include <fmt/printf.h>
#include "RefBuffers.h"

using namespace o2::mch::raw;

std::vector<uint8_t> createGBTBuffer()
{
  BareGBTEncoder::forceNoPhase = true;
  BareGBTEncoder enc(0, 0);
  uint32_t bx(0);
  uint16_t ts(12);
  int elinkId = 0;
  enc.addChannelData(elinkId, 0, {SampaCluster(ts, 10)});
  enc.addChannelData(elinkId, 31, {SampaCluster(ts, 160)});
  elinkId = 3;
  enc.addChannelData(elinkId, 3, {SampaCluster(ts, 13)});
  enc.addChannelData(elinkId, 31, {SampaCluster(ts, 133)});
  enc.addChannelData(elinkId, 13, {SampaCluster(ts, 163)});
  std::vector<uint8_t> words;
  enc.moveToBuffer(words);
  // int i{0};
  // for (auto v : words) {
  //   fmt::printf("0x%02X, ", v);
  //   if (++i % 12 == 0) {
  //     fmt::printf("\n");
  //   }
  // }
  return words;
}

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(gbtencoder)

BOOST_AUTO_TEST_CASE(EncodeABuffer)
{
  auto buffer = createGBTBuffer();
  size_t n = REF_BUFFER_GBT.size();
  BOOST_CHECK_GE(buffer.size(), n);
  BOOST_CHECK(std::equal(begin(buffer), end(buffer), begin(REF_BUFFER_GBT)));
}

BOOST_AUTO_TEST_CASE(BareGBTEncoderLinkIdMustBeBetween0And23)
{
  BOOST_CHECK_THROW(BareGBTEncoder enc(0, 24), std::invalid_argument);
  BOOST_CHECK_NO_THROW(BareGBTEncoder enc(0, 23));
}

BOOST_AUTO_TEST_CASE(BareGBTEncoderAddFewChannels)
{
  BareGBTEncoder enc(0, 0);
  uint32_t bx(0);
  uint16_t ts(0);
  int elinkId = 0;
  enc.addChannelData(elinkId, 0, {SampaCluster(ts, 10)});
  enc.addChannelData(elinkId, 31, {SampaCluster(ts, 160)});
  elinkId = 3;
  enc.addChannelData(elinkId, 3, {SampaCluster(ts, 13)});
  enc.addChannelData(elinkId, 13, {SampaCluster(ts, 133)});
  enc.addChannelData(elinkId, 23, {SampaCluster(ts, 163)});
  BOOST_CHECK_THROW(enc.addChannelData(40, 0, {SampaCluster(ts, 10)}), std::invalid_argument);
  std::vector<uint8_t> buffer;
  enc.moveToBuffer(buffer);
  // we only test >= because the exact size can vary
  // due to the initial phase used in the elinks
  BOOST_CHECK_GE(buffer.size(), 4 * 640);
}

BOOST_AUTO_TEST_CASE(BareGBTEncoderAdd64Channels)
{
  std::vector<uint8_t> buffer;
  BareGBTEncoder enc(0, 0);
  enc.moveToBuffer(buffer);
  uint32_t bx(0);
  uint16_t ts(0);
  int elinkId = 0;
  for (int i = 0; i < 64; i++) {
    enc.addChannelData(elinkId, i % 32, {SampaCluster(ts, i * 10)});
  }
  enc.moveToBuffer(buffer);
  BOOST_CHECK_GE(buffer.size(), 11620); // nof gbt words
}

BOOST_AUTO_TEST_CASE(BareGBTEncoderMoveToBufferClearsTheInternalBuffer)
{
  BareGBTEncoder enc(0, 0);
  enc.addChannelData(0, 0, {SampaCluster(0, 10)});
  std::vector<uint8_t> buffer;
  size_t n = enc.moveToBuffer(buffer);
  BOOST_CHECK_GE(n, 280);
  n = enc.moveToBuffer(buffer);
  BOOST_CHECK_EQUAL(n, 0);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
