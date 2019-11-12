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
#include "BareGBTEncoder.h"
#include "UserLogicGBTEncoder.h"
#include <array>
#include <array>
#include <fmt/printf.h>
#include "RefBuffers.h"
#include <boost/mpl/list.hpp>
#include "MoveBuffer.h"

using namespace o2::mch::raw;

template <typename T, bool CHARGESUM = true>
std::vector<uint8_t> refBufferGBT();

template <>
std::vector<uint8_t> refBufferGBT<BareGBTEncoder, true>()
{
  auto buf = REF_BUFFER_GBT_BARE<true>();
  return std::vector<uint8_t>(buf.begin(), buf.end());
}

template <>
std::vector<uint8_t> refBufferGBT<UserLogicGBTEncoder, true>()
{
  auto buf = REF_BUFFER_GBT_USER_LOGIC<true>();
  return std::vector<uint8_t>(buf.begin(), buf.end());
}

template <typename T, bool CHARGESUM>
std::vector<uint8_t> createGBTBuffer()
{
  T::forceNoPhase = true;
  uint8_t cruId{0};
  uint8_t gbtId{23};
  T enc(cruId, gbtId, CHARGESUM);
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
  // std::cout << "createGBTBuffer<" << typeid(T).name() << "," << std::boolalpha << CHARGESUM << ">\n";
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

typedef boost::mpl::list<BareGBTEncoder, UserLogicGBTEncoder> testTypes;

BOOST_AUTO_TEST_CASE_TEMPLATE(EncodeABufferInChargeSumMode, T, testTypes)
{
  auto buffer = createGBTBuffer<T, true>();
  auto ref = refBufferGBT<T, true>();
  size_t n = ref.size();
  BOOST_CHECK_GE(buffer.size(), n);
  BOOST_CHECK(std::equal(begin(buffer), end(buffer), begin(ref)));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(GBTEncoderCtorLinkIdMustBeBetween0And23, T, testTypes)
{
  BOOST_CHECK_THROW(T enc(0, 24), std::invalid_argument);
  BOOST_CHECK_NO_THROW(T enc(0, 23));
}

template <typename T, bool CHARGESUM>
float expectedSize();

template <>
float expectedSize<BareGBTEncoder, true>()
{
  return 4 * 640;
}

template <>
float expectedSize<UserLogicGBTEncoder, true>()
{
  return 96;
}

BOOST_AUTO_TEST_CASE_TEMPLATE(GBTEncoderAddFewChannels, T, testTypes)
{
  T::forceNoPhase = true;
  T enc(0, 0, true);
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
  float e = expectedSize<T, true>();
  BOOST_CHECK_EQUAL(buffer.size(), e);
}

template <typename T, bool CHARGESUM>
float expectedMaxSize();

template <>
float expectedMaxSize<BareGBTEncoder, true>()
{
  return 4 * 11620;
}

template <>
float expectedMaxSize<UserLogicGBTEncoder, true>()
{
  return 1032;
}

BOOST_AUTO_TEST_CASE_TEMPLATE(GBTEncoderAdd64Channels, T, testTypes)
{
  T::forceNoPhase = true;
  std::vector<uint8_t> buffer;
  T enc(0, 0, true);
  enc.moveToBuffer(buffer);
  uint32_t bx(0);
  uint16_t ts(0);
  int elinkId = 0;
  for (int i = 0; i < 64; i++) {
    enc.addChannelData(elinkId, i % 32, {SampaCluster(ts, i * 10)});
  }
  enc.moveToBuffer(buffer);
  float e = expectedMaxSize<T, true>();
  BOOST_CHECK_EQUAL(buffer.size(), e);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(GBTEncoderMoveToBufferClearsTheInternalBuffer, T, testTypes)
{
  T enc(0, 0, true);
  enc.addChannelData(0, 0, {SampaCluster(0, 10)});
  std::vector<uint8_t> buffer;
  size_t n = enc.moveToBuffer(buffer);
  BOOST_CHECK_GE(n, 0);
  n = enc.moveToBuffer(buffer);
  BOOST_CHECK_EQUAL(n, 0);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
