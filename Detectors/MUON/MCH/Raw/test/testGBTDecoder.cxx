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
#include "MCHRaw/GBT.h"
#include "common.h"
#include "MCHRaw/RAWDataHeader.h"

using namespace o2::mch::raw;
using namespace o2::mch::raw::test;

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(gbtdecoder)

BOOST_AUTO_TEST_CASE(GBTDecoderLinkIdMustBeBetween0And23)
{
  BOOST_CHECK_THROW(GBTDecoder enc(0, 24, handlePacketPrint("dummy")), std::invalid_argument);
  BOOST_CHECK_NO_THROW(GBTDecoder enc(0, 23, handlePacketPrint("dummy")));
}

BOOST_AUTO_TEST_CASE(GBTDecoderFromKnownEncoder)
{
  // here we only test the decoding part.
  // the encoder is assumed to be correct (see testGBTEncoder.cxx)

  std::vector<std::string> result;

  GBTDecoder dec(0, 0, handlePacketStoreAsVec(result));
  auto buffer = o2::mch::raw::test::createGBTBuffer();
  o2::mch::raw::dumpBuffer(buffer);
  for (auto i = 0; i < buffer.size(); i += 4) {
    dec.append(buffer[i], buffer[i + 1],
               buffer[i + 2],
               buffer[i + 3]);
  }
  std::vector<std::string> expected{
    "chip-3-ch-13-ts-12-q-163",
    "chip-3-ch-31-ts-12-q-133",
    "chip-3-ch-3-ts-12-q-13",
    "chip-0-ch-31-ts-12-q-160",
    "chip-0-ch-0-ts-12-q-10"};
  BOOST_CHECK_EQUAL(result.size(), expected.size());
  BOOST_CHECK(std::is_permutation(begin(result), end(result), begin(expected)));
}

BOOST_AUTO_TEST_CASE(GBTDecoderWithSeveralMoveToBuffer)
{
  bool verboseEncoder(false);
  GBTEncoder enc(0, 0);
  uint32_t bx(0);
  uint16_t ts(0);
  std::vector<uint32_t> buffer;
  int elinkId = 2;
  enc.addChannelData(elinkId, 1, {SampaCluster(ts, 10)});
  if (verboseEncoder) {
    std::cout << "before 1st finalize\n";
    enc.printStatus(5);
  }
  enc.moveToBuffer(buffer);
  if (verboseEncoder) {
    std::cout << "after 1st finalize\n";
    enc.printStatus(5);
  }
  enc.addChannelData(elinkId, 2, {SampaCluster(ts, 20)});
  elinkId = 4;
  enc.addChannelData(elinkId, 4, {SampaCluster(ts, 40)});
  enc.addChannelData(elinkId, 5, {SampaCluster(ts, 50)});
  if (verboseEncoder) {
    std::cout << "before 2nd finalize\n";
    enc.printStatus(5);
  }
  enc.moveToBuffer(buffer);
  if (verboseEncoder) {
    std::cout << "after 2nd finalize\n";
    enc.printStatus(5);
  }
  bool verboseDecoder(false);

  std::vector<std::string> result;
  GBTDecoder dec(0, 0, handlePacketStoreAsVec(result));
  for (auto i = 0; i < buffer.size(); i += 4) {
    dec.append(buffer[i], buffer[i + 1],
               buffer[i + 2],
               buffer[i + 3]);
  }
  if (verboseDecoder) {
    dec.printStatus(5);
  }
  std::vector<std::string> expected{
    "chip-2-ch-1-ts-0-q-10",
    "chip-2-ch-2-ts-0-q-20",
    "chip-4-ch-4-ts-0-q-40",
    "chip-4-ch-5-ts-0-q-50"};
  BOOST_CHECK_EQUAL(result.size(), expected.size());
  BOOST_CHECK(std::is_permutation(begin(result), end(result), begin(expected)));
}

BOOST_AUTO_TEST_CASE(GBTDecoderFromBuffer)
{
  std::vector<std::string> result;
  GBTDecoder dec(0, 0, handlePacketStoreAsVec(result));
  for (auto i = 0; i < REF_BUFFER.size(); i += 4) {
    uint32_t w0 = REF_BUFFER[i];
    uint32_t w1 = REF_BUFFER[i + 1];
    uint32_t w2 = REF_BUFFER[i + 2];
    uint32_t w3 = REF_BUFFER[i + 3];
    dec.append(w0, w1, w2, w3);
  }
  std::vector<std::string> expected{
    "chip-3-ch-13-ts-12-q-163",
    "chip-3-ch-31-ts-12-q-133",
    "chip-3-ch-3-ts-12-q-13",
    "chip-0-ch-31-ts-12-q-160",
    "chip-0-ch-0-ts-12-q-10"};
  BOOST_CHECK_EQUAL(result.size(), expected.size());
  BOOST_CHECK(std::is_permutation(begin(result), end(result), begin(expected)));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
