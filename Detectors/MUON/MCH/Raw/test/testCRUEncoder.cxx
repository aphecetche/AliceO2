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

#define BOOST_TEST_MODULE Test MCHRaw CRUEncoder
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include "MCHRaw/CRUEncoder.h"

using namespace o2::mch::raw;

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(cruencoder)

BOOST_AUTO_TEST_CASE(CRUEncoderCtor)
{
  srand(time(nullptr));

  CRUEncoder cru(0);

  uint32_t bx(0);
  uint8_t solarId(0);
  uint8_t elinkId(0);
  uint16_t ts(0);

  cru.addOrbitBC(12345, 123);

  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 0, 10);
  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 16, 160);

  elinkId = 3;

  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 3, 13);
  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 13, 133);
  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 23, 163);

  BOOST_CHECK_GE(cru.len(), 50 + 3 * 90);

  cru.addOrbitBC(12345, 456);

  elinkId = 2;

  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 0, 10);
  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 1, 20);
  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 2, 30);
  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 3, 40);

  elinkId = 10;

  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 22, 420);
  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 23, 430);
  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 24, 440);
  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 25, 450);
  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 26, 460);

  BOOST_CHECK_GE(cru.len(), 50 + 3 * 90 + 5 * 90);

  cru.align();

  cru.gbts2buffer();

  cru.clear();

  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 12, 420);

  cru.printStatus();
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
