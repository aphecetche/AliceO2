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
#include "MCHRaw/GBTEncoder.h"
#include <array>

using namespace o2::mch::raw;

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(gbtencoder)

BOOST_AUTO_TEST_CASE(GBTEncoderLinkIdMustBeBetween0And23)
{
  BOOST_CHECK_THROW(GBTEncoder enc(24), std::invalid_argument);
  BOOST_CHECK_NO_THROW(GBTEncoder enc(23));
}

BOOST_AUTO_TEST_CASE(GBTEncoderAddChannels)
{
  GBTEncoder enc(0);
  uint32_t bx(0);
  uint16_t ts(0);
  int elinkId = 0;
  enc.addChannelChargeSum(bx, elinkId, ts, 0, 10);
  enc.addChannelChargeSum(bx, elinkId, ts, 60, 160);
  elinkId = 3;
  enc.addChannelChargeSum(bx, elinkId, ts, 3, 13);
  enc.addChannelChargeSum(bx, elinkId, ts, 33, 133);
  enc.addChannelChargeSum(bx, elinkId, ts, 63, 163);
  BOOST_CHECK_THROW(enc.addChannelChargeSum(bx, 40, ts, 0, 10), std::invalid_argument);
  enc.finalize();
  // for (auto i = 0; i < enc.size(); i++) {
  //   std::cout << std::hex << "0x" << enc.getWord(i) << "\n";
  // }
  BOOST_CHECK_EQUAL(enc.size(), 160); // nof gbt words
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
