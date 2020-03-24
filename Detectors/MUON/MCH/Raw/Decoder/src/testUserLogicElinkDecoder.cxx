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

#define BOOST_TEST_MODULE Test MCHRaw UserLogicElinkDecoder
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <fstream>
#include <fmt/printf.h>
#include "UserLogicElinkDecoder.h"
#include "MCHRawCommon/SampaHeader.h"
#include "Assertions.h"
#include "MoveBuffer.h"
#include "DumpBuffer.h"
#include "MCHRawCommon/DataFormats.h"
#include "MCHRawDecoder/SampaChannelHandler.h"
#include "MCHRawDecoder/Decoder.h"
#include "PageDecoder.h"
#include "UserLogicEndpointDecoder.h"

using namespace o2::mch::raw;
using o2::header::RAWDataHeaderV4;

using uint10_t = uint16_t;
using uint50_t = uint64_t;

SampaChannelHandler handlePacket(std::string& result)
{
  return [&result](DsElecId dsId, uint8_t channel, SampaCluster sc) {
    result += fmt::format("{}-ch-{}-ts-{}-q", asString(dsId), channel, sc.timestamp);
    if (sc.isClusterSum()) {
      result += fmt::format("-{}", sc.chargeSum);
    } else {
      for (auto s : sc.samples) {
        result += fmt::format("-{}", s);
      }
    }
    result += "\n";
  };
}

SampaHeader createHeader(const std::vector<SampaCluster>& clusters)
{
  uint16_t n10{0};
  for (auto c : clusters) {
    n10 += c.nof10BitWords();
  }
  SampaHeader sh;
  sh.nof10BitWords(n10);
  sh.packetType(SampaPacketType::Data);
  sh.hammingCode(computeHammingCode(sh.uint64()));
  sh.headerParity(computeHeaderParity(sh.uint64()));
  return sh;
}

uint64_t build64(uint16_t a10, uint16_t b10 = 0, uint16_t c10 = 0, uint16_t d10 = 0, uint16_t e10 = 0)
{
  impl::assertIsInRange("a10", a10, 0, 1023);
  impl::assertIsInRange("b10", a10, 0, 1023);
  impl::assertIsInRange("c10", a10, 0, 1023);
  impl::assertIsInRange("d10", a10, 0, 1023);
  impl::assertIsInRange("e10", a10, 0, 1023);
  return (static_cast<uint64_t>(a10) << 40) |
         (static_cast<uint64_t>(b10) << 30) |
         (static_cast<uint64_t>(c10) << 20) |
         (static_cast<uint64_t>(d10) << 10) |
         (static_cast<uint64_t>(e10));
}

void dumpb10(const std::vector<uint10_t>& b10)
{
  for (auto i = 0; i < b10.size(); i++) {
    if (i % 5 == 0) {
      std::cout << "\nB10";
    }
    std::cout << fmt::format("{:4d} ", b10[i]);
  }
  std::cout << "\n";
}

std::vector<uint64_t> b10to64(std::vector<uint10_t> b10, uint16_t prefix14)
{
  uint64_t prefix = prefix14;
  prefix <<= 50;
  std::vector<uint64_t> b64;

  while (b10.size() % 5) {
    b10.emplace_back(0);
  }
  for (auto i = 0; i < b10.size(); i += 5) {
    uint64_t v = build64(b10[i + 4], b10[i + 3], b10[i + 2], b10[i + 1], b10[i + 0]);
    b64.emplace_back(v | prefix);
  }
  return b64;
}

void bufferizeClusters(const std::vector<SampaCluster>& clusters, std::vector<uint10_t>& b10)
{
  for (auto& c : clusters) {
    std::cout << "c=" << c << "\n";
    b10.emplace_back(c.nofSamples());
    b10.emplace_back(c.timestamp);
    if (c.isClusterSum()) {
      b10.emplace_back(c.chargeSum & 0x3FF);
      b10.emplace_back((c.chargeSum & 0xFFC00) >> 10);
    } else {
      for (auto& s : c.samples) {
        b10.emplace_back(s);
      }
    }
  }
}

void append(std::vector<uint10_t>& b10, uint50_t value)
{
  b10.emplace_back((value & 0x3FF));
  b10.emplace_back((value & 0xFFC00) >> 10);
  b10.emplace_back((value & 0x3FF00000) >> 20);
  b10.emplace_back((value & 0xFFC0000000) >> 30);
  b10.emplace_back((value & 0x3FF0000000000) >> 40);
}

std::vector<uint10_t> createBuffer10(const std::vector<SampaCluster>& clusters,
                                     uint8_t chip,
                                     uint8_t ch,
                                     bool sync)
{
  auto sh = createHeader(clusters);
  sh.chipAddress(chip);
  sh.channelAddress(ch);
  std::vector<uint10_t> b10;
  if (sync) {
    append(b10, sampaSyncWord);
  }
  append(b10, sh.uint64());
  bufferizeClusters(clusters, b10);
  return b10;
}

template <typename CHARGESUM>
void decodeBuffer(UserLogicElinkDecoder<CHARGESUM>& dec, const std::vector<uint64_t>& b64)
{
  std::vector<uint8_t> b8;
  impl::copyBuffer(b64, b8);
  impl::dumpBuffer<o2::mch::raw::UserLogicFormat>(b8);
  for (auto b : b64) {
    dec.append(b);
  }
}

constexpr uint8_t chip = 5;
constexpr uint8_t ch = 31;

std::vector<uint10_t> createBuffer10(const std::vector<SampaCluster>& clustersFirstChannel,
                                     const std::vector<SampaCluster>& clustersSecondChannel = {})
{
  bool sync{true};
  auto b10 = createBuffer10(clustersFirstChannel, chip, ch, sync);
  if (clustersSecondChannel.size()) {
    auto chip2 = chip;
    auto ch2 = ch / 2;
    auto b10_2 = createBuffer10(clustersSecondChannel, chip2, ch2, !sync);
    b10.insert(b10.end(), b10_2.begin(), b10_2.end());
  }
  return b10;
}

template <typename CHARGESUM>
std::string testPayloadDecode(const std::vector<SampaCluster>& clustersFirstChannel,
                              const std::vector<SampaCluster>& clustersSecondChannel = {})
{
  auto b10 = createBuffer10(clustersFirstChannel, clustersSecondChannel);
  uint16_t prefix{22}; // 14-bits value.
  // exact value not relevant as long as it is non-zero.
  // Idea being to populate bits 50-63 with some data to ensure
  // the decoder is only using the lower 50 bits to get the sync and
  // header values, for instance.
  auto b64 = b10to64(b10, prefix);

  std::string results;

  uint16_t dummySolarId{0};
  uint8_t dummyGroup{0};
  uint8_t index = (chip - (ch > 32)) / 2;
  DsElecId dsId{dummySolarId, dummyGroup, index};
  UserLogicElinkDecoder<CHARGESUM> dec(dsId, handlePacket(results));
  decodeBuffer(dec, b64);
  return results;
}

template <typename CHARGESUM>
void testDecode(const std::vector<SampaCluster>& clustersFirstChannel,
                const std::vector<SampaCluster>& clustersSecondChannel = {})
{
  auto b10 = createBuffer10(clustersFirstChannel, clustersSecondChannel);
  uint16_t prefix{22};
  auto b64 = b10to64(b10, prefix);

  std::vector<uint8_t> b8;
  impl::copyBuffer(b64, b8);

  std::vector<uint8_t> buffer;
  auto rdh = createRDH<RAWDataHeaderV4>(0, 0, 0, 12, 34, b8.size());
  appendRDH(buffer, rdh);
  buffer.insert(buffer.end(), b8.begin(), b8.end());

  const auto handlePacket = [](DsElecId dsId, uint8_t channel, SampaCluster sc) {
    std::cout << fmt::format("testDecode:{}-{}\n", asString(dsId), asString(sc));
  };

  PageDecoder<RAWDataHeaderV4, UserLogicFormat, UserLogicEndpointDecoder<CHARGESUM>> decoder(handlePacket);

  decoder(buffer);
}

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(userlogicdsdecoder)

BOOST_AUTO_TEST_CASE(SampleModeSimplest)
{
  // only one channel with one very small cluster
  // fitting within one 64-bits word
  SampaCluster cl(345, {123, 456});
  auto r = testPayloadDecode<SampleMode>({cl});
  BOOST_CHECK_EQUAL(r, "S0-J0-DS2-ch-63-ts-345-q-123-456\n");
}

BOOST_AUTO_TEST_CASE(DecodeSampleModeSimplest)
{
  // only one channel with one very small cluster
  // fitting within one 64-bits word
  SampaCluster cl(345, {123, 456});
  testDecode<SampleMode>({cl});
  BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(SampleModeSimple)
{
  // only one channel with one cluster, but the cluster
  // spans 2 64-bits words.
  SampaCluster cl(345, {123, 456, 789, 901, 902});
  auto r = testPayloadDecode<SampleMode>({cl});
  BOOST_CHECK_EQUAL(r, "S0-J0-DS2-ch-63-ts-345-q-123-456-789-901-902\n");
}

BOOST_AUTO_TEST_CASE(SampleModeTwoChannels)
{
  // 2 channels with one cluster
  SampaCluster cl(345, {123, 456, 789, 901, 902});
  SampaCluster cl2(346, {1001, 1002, 1003, 1004, 1005, 1006, 1007});
  auto r = testPayloadDecode<SampleMode>({cl}, {cl2});
  BOOST_CHECK_EQUAL(r,
                    "S0-J0-DS2-ch-63-ts-345-q-123-456-789-901-902\n"
                    "S0-J0-DS2-ch-47-ts-346-q-1001-1002-1003-1004-1005-1006-1007\n");
}

BOOST_AUTO_TEST_CASE(ChargeSumModeSimplest)
{
  // only one channel with one cluster
  // (hence fitting within one 64 bits word)
  SampaCluster cl(345, 123456);
  auto r = testPayloadDecode<ChargeSumMode>({cl});
  BOOST_CHECK_EQUAL(r, "S0-J0-DS2-ch-63-ts-345-q-123456\n");
}

BOOST_AUTO_TEST_CASE(ChargeSumModeSimple)
{
  // only one channel with 2 clusters
  // (hence spanning 2 64-bits words)
  SampaCluster cl1(345, 123456);
  SampaCluster cl2(346, 789012);
  auto r = testPayloadDecode<ChargeSumMode>({cl1, cl2});
  BOOST_CHECK_EQUAL(r,
                    "S0-J0-DS2-ch-63-ts-345-q-123456\n"
                    "S0-J0-DS2-ch-63-ts-346-q-789012\n");
}

BOOST_AUTO_TEST_CASE(ChargeSumModeTwoChannels)
{
  // two channels with 2 clusters
  SampaCluster cl1(345, 123456);
  SampaCluster cl2(346, 789012);
  SampaCluster cl3(347, 1357);
  SampaCluster cl4(348, 791);
  auto r = testPayloadDecode<ChargeSumMode>({cl1, cl2}, {cl3, cl4});
  BOOST_CHECK_EQUAL(r,
                    "S0-J0-DS2-ch-63-ts-345-q-123456\n"
                    "S0-J0-DS2-ch-63-ts-346-q-789012\n"
                    "S0-J0-DS2-ch-47-ts-347-q-1357\n"
                    "S0-J0-DS2-ch-47-ts-348-q-791\n");
}

// BOOST_AUTO_TEST_CASE(TestRecoverableError)
// {
//   const auto channelHandler = [](o2::mch::raw::DsElecId dsId,
//                                  uint8_t channel,
//                                  o2::mch::raw::SampaCluster) {
//     std::cout << "channelHandler called !\n";
//   };
//
//   constexpr uint64_t sampaSyncWord{0x1555540f00113};
//
//   o2::mch::raw::UserLogicElinkDecoder<SampleMode> ds{DsElecId{0, 0, 2}, channelHandler};
//
//   ds.append(sampaSyncWord);
//   ds.append(0x1722e9f00327d);
//   ds.append(1);
//   ds.append(2);
//
//   ds.status();
//   BOOST_CHECK(true);
// }

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
