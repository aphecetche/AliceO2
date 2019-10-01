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
  CRUEncoder cru(0);

  uint32_t bx(0);
  uint8_t solarId(0);
  uint8_t elinkId(0);
  uint16_t ts(0);

  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 0, 10);
  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 60, 160);

  elinkId = 3;

  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 3, 13);
  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 33, 133);
  cru.addChannelChargeSum(bx, solarId, elinkId, ts, 63, 163);

  cru.finalize();

  cru.printStatus();
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
