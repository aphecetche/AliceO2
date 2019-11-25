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

using namespace o2::mch::raw;

SampaChannelHandler handlePacket(std::string& result)
{
  return [&result](uint8_t cruId, uint8_t linkId, uint8_t chip, uint8_t channel, SampaCluster sc) {
    result += fmt::format("chip-{}-ch-{}-ts-{}-q", chip, channel, sc.timestamp);
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

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(userlogicdsdecoder)

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

SampaHeader createHeader(std::vector<SampaCluster> clusters)
{
  uint16_t n10{0};
  for (auto c : clusters) {
    n10 += c.nof10BitWords();
  }
  SampaHeader sh;
  sh.nof10BitWords(n10);
  sh.packetType(SampaPacketType::Data);
  sh.hammingCode(computeHammingCode(sh.uint64()));
  return sh;
}

void append(uint64_t prefix, std::vector<uint64_t>& buffer, uint8_t& index, uint64_t& word, int data)
{
  word |= static_cast<uint64_t>(data) << (index * 10);
  if (index == 0) {
    buffer.emplace_back(prefix | word);
    index = 4;
    word = 0;
  } else {
    --index;
  }
}

void bufferizeClusters(const std::vector<SampaCluster>& clusters,
                       std::vector<uint64_t>& b64,
                       const uint64_t prefix = 0)
{
  impl::assertIsInRange("prefix", prefix, 0, 0x3FFFFFFFFFFFF);
  uint64_t word{0};
  uint8_t index{4};
  for (auto& c : clusters) {
    std::cout << "c=" << c << "\n";
    append(prefix, b64, index, word, c.nofSamples());
    append(prefix, b64, index, word, c.timestamp);
    if (c.isClusterSum()) {
      append(prefix, b64, index, word, c.chargeSum & 0x3FF);
      append(prefix, b64, index, word, (c.chargeSum & 0xFFC00) >> 10);
    } else {
      for (auto& s : c.samples) {
        append(prefix, b64, index, word, s);
      }
    }
  }
  while (index != 4) {
    append(prefix, b64, index, word, 0);
  }
}

std::vector<uint64_t> createBuffer(const std::vector<SampaCluster>& clusters)
{
  auto sh = createHeader(clusters);
  std::vector<uint64_t> b64{sampaSyncWord, sh.uint64()};
  bufferizeClusters(clusters, b64);
  return b64;
}

template <typename CHARGESUM>
void decodeBuffer(UserLogicElinkDecoder<CHARGESUM>& dec, const std::vector<uint64_t>& b64)
{
  std::vector<uint8_t> b8;
  impl::copyBuffer(b64, b8);
  impl::dumpBuffer(b8);
  for (auto b : b64) {
    dec.append(b);
  }
}

template <typename CHARGESUM>
std::string testDecode(const std::vector<SampaCluster>& clusters)
{
  std::string results;
  UserLogicElinkDecoder<CHARGESUM> dec(0, 0, handlePacket(results));
  auto b64 = createBuffer(clusters);
  decodeBuffer(dec, b64);
  return results;
}

BOOST_AUTO_TEST_CASE(SampleModeSimplest)
{
  // only one channel with one very small cluster
  // fitting within one 64-bits word
  SampaCluster cl(345, {123, 456});
  auto r = testDecode<SampleMode>({cl});
  BOOST_CHECK_EQUAL(r, "chip-0-ch-0-ts-345-q-123-456\n");
}

BOOST_AUTO_TEST_CASE(SampleModeSimple)
{
  // only one channel with one cluster, but the cluster
  // spans 2 64-bits words.
  SampaCluster cl(345, {123, 456, 789, 901, 902});
  auto r = testDecode<SampleMode>({cl});
  BOOST_CHECK_EQUAL(r, "chip-0-ch-0-ts-345-q-123-456-789-901-902\n");
}

BOOST_AUTO_TEST_CASE(ChargeSumModeSimplest)
{
  // only one channel with one cluster
  // (hence fitting within one 64 bits word)
  SampaCluster cl(345, 123456);
  auto r = testDecode<ChargeSumMode>({cl});
  BOOST_CHECK_EQUAL(r, "chip-0-ch-0-ts-345-q-123456\n");
}

BOOST_AUTO_TEST_CASE(ChargeSumModeSimple)
{
  // only one channel with 2 clusters
  // (hence spanning 2 64-bits words)
  SampaCluster cl1(345, 123456);
  SampaCluster cl2(346, 789012);
  auto r = testDecode<ChargeSumMode>({cl1, cl2});
  BOOST_CHECK_EQUAL(r,
                    "chip-0-ch-0-ts-345-q-123456\n"
                    "chip-0-ch-0-ts-346-q-789012\n");
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
