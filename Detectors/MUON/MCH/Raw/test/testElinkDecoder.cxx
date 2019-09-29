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

#define BOOST_TEST_MODULE Test MCHRaw ElinkDecoder
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <fstream>
#include <fmt/printf.h>
#include "MCHRaw/ElinkDecoder.h"
#include "common.h"

using namespace o2::mch::raw;

void handlePacket(uint8_t chip, uint8_t channel, uint16_t timetamp,
                  uint32_t chargeSum)
{
  std::cout << " chip= " << (int)chip << " ch= " << (int)channel << " ts=" << (int)timetamp << " q=" << (int)chargeSum
            << "\n";
}

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(elinkdecoder)

BOOST_AUTO_TEST_CASE(TestELinkDecoding)
{
  ElinkEncoder enc = encoderExample1();
  int npackets{0};

  auto hp = [&npackets](uint8_t chip, uint8_t channel, uint16_t timestamp,
                        uint32_t chargeSum) {
    npackets++;
    handlePacket(chip, channel, timestamp, chargeSum);
  };

  ElinkDecoder e(enc.id(), hp);

  for (int i = 0; i < enc.len() - 1; i += 2) {
    e.append(enc.get(i), enc.get(i + 1));
  }

  std::cout << "nof bits : " << enc.len() << "\n";
  std::cout << "elink=" << e << "\n";
  std::cout << "npackets=" << npackets << "\n";
  BOOST_CHECK_EQUAL(npackets, 4);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
