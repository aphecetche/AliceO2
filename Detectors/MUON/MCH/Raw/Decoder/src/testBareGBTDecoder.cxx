// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#define BOOST_TEST_MODULE Test MCHRaw BareGBTDecoder
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include "BareGBTDecoder.h"
#include <array>
#include <fmt/format.h>
#include <array>
#include "MCHRawCommon/RDHManip.h"
#include "MCHRawCommon/DataFormats.h"
#include "RefBuffers.h"
#include "DumpBuffer.h"

using namespace o2::mch::raw;

SampaChannelHandler handlePacketPrint(std::string_view msg)
{
  return [msg](uint8_t cruId, uint8_t linkId, uint8_t chip, uint8_t channel, SampaCluster sc) {
    std::cout << fmt::format("{}chip={:2d} ch={:2d} ", msg, chip, channel);
    std::cout << sc << "\n";
  };
}

SampaChannelHandler handlePacketStoreAsVec(std::vector<std::string>& result)
{
  return [&result](uint8_t cruId, uint8_t linkId, uint8_t chip, uint8_t channel, SampaCluster sc) {
    result.emplace_back(fmt::format("chip-{}-ch-{}-ts-{}-q-{}", chip, channel, sc.timestamp, sc.chargeSum));
  };
}

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(gbtdecoder)

BOOST_AUTO_TEST_CASE(BareGBTDecoderLinkIdMustBeBetween0And23)
{
  BOOST_CHECK_THROW(BareGBTDecoder decoder(0, 24, handlePacketPrint("dummy")), std::invalid_argument);
  BOOST_CHECK_NO_THROW(BareGBTDecoder decoder(0, 23, handlePacketPrint("dummy")));
}

BOOST_AUTO_TEST_CASE(BareGBTDecoderFromKnownEncoder)
{
  // here we only test the decoding part.
  // the encoder is assumed to be correct (see testGBTEncoder.cxx)

  std::vector<std::string> result;

  BareGBTDecoder dec(0, 0, handlePacketStoreAsVec(result));
  auto buf = REF_BUFFER_GBT<BareFormat, ChargeSumMode>();
  gsl::span<uint8_t> buffer(buf);
  impl::dumpBuffer(buffer);
  dec.append(buffer);
  std::vector<std::string> expected{
    "chip-3-ch-13-ts-12-q-163",
    "chip-3-ch-31-ts-12-q-133",
    "chip-3-ch-3-ts-12-q-13",
    "chip-0-ch-31-ts-12-q-160",
    "chip-0-ch-0-ts-12-q-10"};
  BOOST_CHECK_EQUAL(result.size(), expected.size());
  if (result.size()) {
    for (auto r : result) {
      std::cout << "r=" << r << "\n";
    }
  }
  BOOST_CHECK(std::is_permutation(begin(result), end(result), begin(expected)));
}

BOOST_AUTO_TEST_CASE(BareGBTDecoderFromBuffer)
{
  std::vector<std::string> result;
  BareGBTDecoder dec(0, 0, handlePacketStoreAsVec(result));
  auto buf = REF_BUFFER_GBT<BareFormat, ChargeSumMode>();
  dec.append(buf);
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
