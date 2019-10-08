// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#define BOOST_TEST_MODULE Test MCHRaw Decoder
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <fstream>
#include <fmt/printf.h>
#include "MCHRaw/Decoder.h"
#include "MCHRaw/GBTDecoder.h"
#include "common.h"

using namespace o2::mch::raw;

void handlePacket(uint8_t chip, uint8_t channel, uint16_t timetamp,
                  uint32_t chargeSum)
{
  std::cout << " chip= " << (int)chip << " ch= " << (int)channel << " ts=" << (int)timetamp << " q=" << (int)chargeSum
            << "\n";
}

bool handleRDH(const RAWDataHeader& rdh)
{
  // std::cout << std::string(80, '-') << "\n";
  // std::cout << rdh << "\n";
  return true;
}

SampaChannelHandler handlePacketCompact(std::vector<std::string>& result)
{
  return [&result](uint8_t chip, uint8_t channel, uint16_t timestamp,
                   uint32_t chargeSum) {
    result.emplace_back(fmt::format("chip-{}-ch-{}-ts-{}-q-{}", chip, channel, timestamp, chargeSum));
  };
}

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(decoder)

BOOST_AUTO_TEST_CASE(Test0)
{
  RawDataHeaderHandler rh;
  SampaChannelHandler ch;

  auto d = createBareDecoder(rh, ch);

  createBareDecoder(handleRDH, ch);
  createBareDecoder(handleRDH, handlePacket);
}

BOOST_AUTO_TEST_CASE(Test1)
{
  auto buffer = o2::mch::raw::test::createGBTBuffer();

  auto rdh = o2::mch::raw::createRDH(0, 0, 12, 34, buffer.size() * 4);
  std::vector<uint32_t> testBuffer;

  // duplicate the (rdh+payload) to fake a 3 rdhs buffer
  int nrdhs{3};
  for (int i = 0; i < nrdhs; i++) {
    o2::mch::raw::appendRDH(testBuffer, rdh);
    std::copy(begin(buffer), end(buffer), std::back_inserter(testBuffer));
  }

  int n = o2::mch::raw::test::countRDHs(testBuffer);

  // o2::mch::raw::test::dumpBuffer(testBuffer);
  BOOST_CHECK_EQUAL(n, nrdhs);

  o2::mch::raw::test::showRDHs(testBuffer);
}

BOOST_AUTO_TEST_CASE(TestDecoding)
{
  auto testBuffer = o2::mch::raw::test::createCRUBuffer();
  int n = o2::mch::raw::test::countRDHs(testBuffer);
  BOOST_CHECK_EQUAL(n, 4);
  std::vector<std::string> result;
  std::vector<std::string> expected{
    "chip-0-ch-0-ts-0-q-10",
    "chip-2-ch-0-ts-0-q-10",
    "chip-3-ch-0-ts-0-q-10",
    "chip-3-ch-3-ts-0-q-13",
    "chip-3-ch-13-ts-0-q-133",
    "chip-3-ch-23-ts-0-q-163",
    "chip-2-ch-0-ts-0-q-10",
    "chip-2-ch-1-ts-0-q-20",
    "chip-2-ch-2-ts-0-q-30",
    "chip-2-ch-3-ts-0-q-40",
    "chip-10-ch-22-ts-0-q-420",
    "chip-10-ch-23-ts-0-q-430",
    "chip-10-ch-24-ts-0-q-440",
    "chip-10-ch-25-ts-0-q-450",
    "chip-10-ch-26-ts-0-q-460",
    "chip-10-ch-12-ts-0-q-420"};

  auto decode = o2::mch::raw::createBareDecoder(handleRDH, handlePacketCompact(result));
  decode(testBuffer);

  BOOST_CHECK_EQUAL(result.size(), expected.size());
  BOOST_CHECK(std::is_permutation(begin(result), end(result), begin(expected)));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
