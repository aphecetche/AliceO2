// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#define BOOST_TEST_MODULE Test MCHRaw RAWDataHeader
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include "MCHRaw/RAWDataHeader.h"
#include "common.h"

using namespace o2::mch::raw;

std::vector<uint32_t> testBuffer()
{
  std::vector<uint32_t> buffer(16);
  int n{0};
  for (int i = 0; i < 16; i++) {
    buffer[i] = n | ((n + 1) << 8) | ((n + 2) << 16) | ((n + 3) << 24);
    n += 4;
  }
  return buffer;
}

std::vector<uint8_t> testBuffer8()
{
  std::vector<uint8_t> buffer(64);
  for (int i = 0; i < 64; i++) {
    buffer[i] = i;
  }
  return buffer;
}

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(rawdataheader)

BOOST_AUTO_TEST_CASE(AppendRDH)
{
  RAWDataHeader rdh;
  rdh.word3 = 0x03020100;
  rdh.word2 = 0x07060504;
  rdh.word1 = 0x0B0A0908;
  rdh.word0 = 0x0F0E0D0C;
  rdh.word7 = 0x13121110;
  rdh.word6 = 0x17161514;
  rdh.word5 = 0x1B1A1918;
  rdh.word4 = 0x1F1E1D1C;
  rdh.word11 = 0x23222120;
  rdh.word10 = 0x27262524;
  rdh.word9 = 0x2B2A2928;
  rdh.word8 = 0x2F2E2D2C;
  rdh.word15 = 0x33323130;
  rdh.word14 = 0x37363534;
  rdh.word13 = 0x3B3A3938;
  rdh.word12 = 0x3F3E3D3C;
  std::vector<uint32_t> buffer;
  appendRDH(buffer, rdh);
  auto tb = testBuffer();
  BOOST_CHECK_EQUAL(buffer.size(), tb.size());
  BOOST_CHECK(std::equal(begin(buffer), end(buffer), begin(tb)));
}

BOOST_AUTO_TEST_CASE(AppendRDH8)
{
  RAWDataHeader rdh;
  rdh.word3 = 0x03020100;
  rdh.word2 = 0x07060504;
  rdh.word1 = 0x0B0A0908;
  rdh.word0 = 0x0F0E0D0C;
  rdh.word7 = 0x13121110;
  rdh.word6 = 0x17161514;
  rdh.word5 = 0x1B1A1918;
  rdh.word4 = 0x1F1E1D1C;
  rdh.word11 = 0x23222120;
  rdh.word10 = 0x27262524;
  rdh.word9 = 0x2B2A2928;
  rdh.word8 = 0x2F2E2D2C;
  rdh.word15 = 0x33323130;
  rdh.word14 = 0x37363534;
  rdh.word13 = 0x3B3A3938;
  rdh.word12 = 0x3F3E3D3C;
  std::vector<uint8_t> buffer;
  appendRDH(buffer, rdh);
  auto tb = testBuffer8();
  BOOST_CHECK_EQUAL(buffer.size(), tb.size());
  BOOST_CHECK(std::equal(begin(buffer), end(buffer), begin(tb)));
}

BOOST_AUTO_TEST_CASE(CreateRDHFromBuffer)
{
  auto buffer = testBuffer();

  auto rdh = createRDH(buffer);
  BOOST_CHECK_EQUAL(rdh.word3, 0x03020100);
  BOOST_CHECK_EQUAL(rdh.word2, 0x07060504);
  BOOST_CHECK_EQUAL(rdh.word1, 0x0B0A0908);
  BOOST_CHECK_EQUAL(rdh.word0, 0x0F0E0D0C);
  BOOST_CHECK_EQUAL(rdh.word7, 0x13121110);
  BOOST_CHECK_EQUAL(rdh.word6, 0x17161514);
  BOOST_CHECK_EQUAL(rdh.word5, 0x1B1A1918);
  BOOST_CHECK_EQUAL(rdh.word4, 0x1F1E1D1C);
  BOOST_CHECK_EQUAL(rdh.word11, 0x23222120);
  BOOST_CHECK_EQUAL(rdh.word10, 0x27262524);
  BOOST_CHECK_EQUAL(rdh.word9, 0x2B2A2928);
  BOOST_CHECK_EQUAL(rdh.word8, 0x2F2E2D2C);
  BOOST_CHECK_EQUAL(rdh.word15, 0x33323130);
  BOOST_CHECK_EQUAL(rdh.word14, 0x37363534);
  BOOST_CHECK_EQUAL(rdh.word13, 0x3B3A3938);
  BOOST_CHECK_EQUAL(rdh.word12, 0x3F3E3D3C);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
