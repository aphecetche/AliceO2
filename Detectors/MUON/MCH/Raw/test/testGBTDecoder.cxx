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

using namespace o2::mch::raw;

void handlePacket(uint8_t chip, uint8_t channel, uint16_t timetamp,
                  uint32_t chargeSum)
{
  std::cout << "Decoder callback got: chip= " << (int)chip << " ch= " << (int)channel << " ts=" << (int)timetamp << " q=" << (int)chargeSum
            << "\n";
}

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(gbtdecoder)

BOOST_AUTO_TEST_CASE(GBTDecoderLinkIdMustBeBetween0And23)
{
  BOOST_CHECK_THROW(GBTDecoder enc(24, handlePacket), std::invalid_argument);
  BOOST_CHECK_NO_THROW(GBTDecoder enc(23, handlePacket));
}

BOOST_AUTO_TEST_CASE(GBTDecoderFromKnownEncoder)
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
  int expectedSize = enc.len() / 2;
  enc.finalize();
  BOOST_CHECK_EQUAL(enc.size(), expectedSize); // nof gbt words

  GBTDecoder dec(0, handlePacket);
  for (auto i = 0; i < enc.size(); i++) {
    dec.append(enc.getWord(i));
  }
}

BOOST_AUTO_TEST_CASE(GBTDecoderFromKnownEncoderWithAdditionAfterFinalize)
{
  GBTEncoder enc(0);
  uint32_t bx(0);
  uint16_t ts(0);
  int elinkId = 4;
  enc.addChannelChargeSum(bx, elinkId, ts, 1, 10);
  enc.printStatus();
  enc.finalize();
  std::cout << "\nafter first finalize\n";
  enc.printStatus();
  enc.addChannelChargeSum(bx, elinkId, ts, 2, 20);
  enc.printStatus();
  enc.finalize();
  std::cout << "\nafter second finalize\n";
  enc.printStatus();

  GBTDecoder dec(0, handlePacket);
  for (auto i = 0; i < enc.size(); i++) {
    dec.append(enc.getWord(i));
  }
  dec.printStatus();
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
