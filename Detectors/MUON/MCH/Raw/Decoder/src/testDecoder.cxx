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
#include "MCHRawCommon/RDHManip.h"
#include "MCHRawCommon/DataFormats.h"
#include "MCHRawDecoder/Decoder.h"
#include "Headers/RAWDataHeader.h"
#include "RefBuffers.h"

using namespace o2::mch::raw;
using namespace o2::mch::raw::dataformat;
using o2::header::RAWDataHeaderV4;

std::ostream& operator<<(std::ostream&, const RAWDataHeaderV4&);

bool handleRDH(const RAWDataHeaderV4& rdh)
{
  // std::cout << std::string(80, '-') << "\n";
  // std::cout << rdh << "\n";
  return true;
}

SampaChannelHandler handlePacketStoreAsVec(std::vector<std::string>& result)
{
  return [&result](uint8_t cruId, uint8_t linkId, uint8_t chip, uint8_t channel, SampaCluster sc) {
    result.emplace_back(fmt::format("chip-{}-ch-{}-ts-{}-q-{}", chip, channel, sc.timestamp, sc.chargeSum));
  };
}

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(decoder)

BOOST_AUTO_TEST_CASE(Test0)
{
  RawDataHeaderHandler<RAWDataHeaderV4> rh;
  SampaChannelHandler ch;

  auto d = createBareDecoder(rh, ch, true);

  createBareDecoder<RAWDataHeaderV4>(handleRDH, ch, true);
  createBareDecoder<RAWDataHeaderV4>(
    handleRDH, [](uint8_t cruId, uint8_t linkId, uint8_t chip, uint8_t channel, SampaCluster sc) {
    },
    false);
}

BOOST_AUTO_TEST_CASE(Test1)
{
  std::vector<uint8_t> buffer(1024);
  std::generate(buffer.begin(), buffer.end(), std::rand);

  auto rdh = createRDH<RAWDataHeaderV4>(0, 0, 12, 34, buffer.size());

  std::vector<uint8_t> testBuffer;

  // duplicate the (rdh+payload) to fake a 3 rdhs buffer
  int nrdhs{3};
  for (int i = 0; i < nrdhs; i++) {
    appendRDH(testBuffer, rdh);
    std::copy(begin(buffer), end(buffer), std::back_inserter(testBuffer));
  }

  int n = countRDHs<RAWDataHeaderV4>(testBuffer);

  BOOST_CHECK_EQUAL(n, nrdhs);

  showRDHs<RAWDataHeaderV4>(testBuffer);
}

BOOST_AUTO_TEST_CASE(TestDecoding)
{
  auto testBuffer = REF_BUFFER_CRU<Bare, ChargeSumMode>();
  int n = countRDHs<RAWDataHeaderV4>(testBuffer);
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

  auto decode = createBareDecoder<RAWDataHeaderV4>(handleRDH, handlePacketStoreAsVec(result), true);
  decode(testBuffer);

  BOOST_CHECK_EQUAL(result.size(), expected.size());
  BOOST_CHECK(std::is_permutation(begin(result), end(result), begin(expected)));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
