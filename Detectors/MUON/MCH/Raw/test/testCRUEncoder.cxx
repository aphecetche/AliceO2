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

  std::cout << "\nSTEP 0\n";
  cru.printStatus();

  cru.addOrbitBC(12345, 123);

  std::cout << "\nSTEP 1\n";
  cru.printStatus();

  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 0, 10);
  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 60, 160);

  elinkId = 3;

  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 3, 13);
  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 33, 133);
  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 63, 163);

  BOOST_CHECK_GE(cru.len(), 50 + 3 * 90);

  std::cout << "\nSTEP 2\n";
  cru.printStatus();

  cru.addOrbitBC(12345, 456);

  std::cout << "\nSTEP 3\n";
  cru.printStatus();

  elinkId = 2;

  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 0, 10);
  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 1, 20);
  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 2, 30);
  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 3, 40);

  elinkId = 10;

  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 42, 420);
  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 43, 430);
  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 44, 440);
  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 45, 450);
  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 46, 460);

  BOOST_CHECK_GE(cru.len(), 50 + 3 * 90 + 5 * 90);

  std::cout << "\nSTEP 4\n";
  cru.printStatus();

  cru.align();

  std::cout << "\nSTEP 5\n";
  cru.printStatus();

  cru.gbts2buffer();

  std::cout << "\nSTEP 6\n";
  cru.printStatus();

  cru.clear();

  std::cout << "\nSTEP 7(after clear\n";
  cru.printStatus();

  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 42, 420);

  std::cout << "\nSTEP 8\n";
  cru.printStatus();
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
