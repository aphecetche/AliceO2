// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#define BOOST_TEST_MODULE Test MCHRaw GBTDecoder
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include "MCHRaw/GBTDecoder.h"
#include "MCHRaw/GBTEncoder.h"
#include <array>
#include <fmt/format.h>

using namespace o2::mch::raw;

SampaChannelHandler handlePacket(std::string_view msg)
{
  return [msg](uint8_t chip, uint8_t channel, uint16_t timestamp,
               uint32_t chargeSum) {
    std::cout << msg << ": Decoder callback got: chip= " << (int)chip << " ch= " << (int)channel << " ts=" << (int)timestamp << " q=" << (int)chargeSum
              << "\n";
  };
}

SampaChannelHandler handlePacketCompact(std::vector<std::string>& result)
{
  return [&result](uint8_t chip, uint8_t channel, uint16_t timestamp,
                   uint32_t chargeSum) {
    result.emplace_back(fmt::format("chip-{}-ch-{}-ts-{}-q-{}", chip, channel, timestamp, chargeSum));
  };
}

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(gbtdecoder)

BOOST_AUTO_TEST_CASE(GBTDecoderLinkIdMustBeBetween0And23)
{
  BOOST_CHECK_THROW(GBTDecoder enc(24, handlePacket("dummy")), std::invalid_argument);
  BOOST_CHECK_NO_THROW(GBTDecoder enc(23, handlePacket("dummy")));
}

BOOST_AUTO_TEST_CASE(GBTDecoderFromKnownEncoder)
{
  GBTEncoder enc(0);
  uint32_t bx(0);
  uint16_t ts(12);
  int elinkId = 0;
  enc.addChannelChargeSum(bx, elinkId, ts, 0, 10);
  enc.addChannelChargeSum(bx, elinkId, ts, 31, 160);
  elinkId = 3;
  enc.addChannelChargeSum(bx, elinkId, ts, 3, 13);
  enc.addChannelChargeSum(bx, elinkId, ts, 31, 133);
  enc.addChannelChargeSum(bx, elinkId, ts, 13, 163);
  BOOST_CHECK_THROW(enc.addChannelChargeSum(bx, 40, ts, 0, 10), std::invalid_argument);
  int expectedSize = enc.len() / 2;
  enc.finalize();
  BOOST_CHECK_EQUAL(enc.size(), expectedSize); // nof gbt words

  std::vector<std::string> result;
  GBTDecoder dec(0, handlePacketCompact(result));
  for (auto i = 0; i < enc.size(); i++) {
    dec.append(enc.getWord(i));
  }
  dec.finalize();
  std::vector<std::string> expected{
    "chip-3-ch-13-ts-12-q-163",
    "chip-3-ch-31-ts-12-q-133",
    "chip-3-ch-3-ts-12-q-13",
    "chip-0-ch-31-ts-12-q-160",
    "chip-0-ch-0-ts-12-q-10"};
  BOOST_CHECK(std::is_permutation(begin(result), end(result), begin(expected)));
}

BOOST_AUTO_TEST_CASE(GBTDecoderWithAdditionAfterFinalize)
{
  bool verboseEncoder(false);
  GBTEncoder enc(0);
  uint32_t bx(0);
  uint16_t ts(0);
  int elinkId = 2;
  enc.addChannelChargeSum(bx, elinkId, ts, 1, 10);
  if (verboseEncoder) {
    std::cout << "before 1st finalize\n";
    enc.printStatus(5);
  }
  enc.finalize();
  if (verboseEncoder) {
    std::cout << "after 1st finalize\n";
    enc.printStatus(5);
  }
  enc.addChannelChargeSum(bx, elinkId, ts, 2, 20);
  elinkId = 4;
  enc.addChannelChargeSum(bx, elinkId, ts, 4, 40);
  enc.addChannelChargeSum(bx, elinkId, ts, 5, 50);
  if (verboseEncoder) {
    std::cout << "before 2nd finalize\n";
    enc.printStatus(5);
  }
  enc.finalize();
  if (verboseEncoder) {
    std::cout << "after 2nd finalize\n";
    enc.printStatus(5);
  }
  bool verboseDecoder(false);

  std::vector<std::string> result;
  GBTDecoder dec(0, handlePacketCompact(result));
  for (auto i = 0; i < enc.size(); i++) {
    dec.append(enc.getWord(i));
  }
  dec.finalize();
  if (verboseDecoder) {
    dec.printStatus(5);
  }
  std::vector<std::string> expected{
    "chip-2-ch-1-ts-0-q-10",
    "chip-2-ch-2-ts-0-q-20",
    "chip-4-ch-4-ts-0-q-40",
    "chip-4-ch-5-ts-0-q-50"};
  BOOST_CHECK(std::is_permutation(begin(result), end(result), begin(expected)));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
