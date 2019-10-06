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

BOOST_AUTO_TEST_CASE(StartHBFrameBunchCrossingMustBe12Bits)
{
  CRUEncoder cru(0);
  BOOST_CHECK_THROW(cru.startHeartbeatFrame(0, 1 << 12), std::invalid_argument);
  BOOST_CHECK_NO_THROW(cru.startHeartbeatFrame(0, 0xFFF));
}

BOOST_AUTO_TEST_CASE(EmptyEncoderHasEmptyBuffer)
{
  srand(time(nullptr));
  CRUEncoder cru(0);
  cru.startHeartbeatFrame(12345, 123);
  std::vector<uint32_t> buffer;
  cru.moveToBuffer(buffer);
  BOOST_CHECK_EQUAL(buffer.size(), 0);
}

BOOST_AUTO_TEST_CASE(MultipleOrbitsWithNoDataIsAnEmptyBuffer)
{
  srand(time(nullptr));
  CRUEncoder cru(0);
  cru.startHeartbeatFrame(12345, 123);
  cru.startHeartbeatFrame(12345, 125);
  cru.startHeartbeatFrame(12345, 312);
  std::vector<uint32_t> buffer;
  cru.moveToBuffer(buffer);
  BOOST_CHECK_EQUAL(buffer.size(), 0);
}

BOOST_AUTO_TEST_CASE(CheckNumberOfRDHs)
{
  srand(time(nullptr));

  CRUEncoder cru(0);

  uint32_t bx(0);
  uint8_t solarId(0);
  uint8_t elinkId(0);
  uint16_t ts(0);

  cru.startHeartbeatFrame(12345, 123);

  cru.addChannelChargeSum(solarId, elinkId, ts, 0, 10);

  std::cout << ">>> After 1 channel\n";
  cru.printStatus(1);
  std::cout << "<<< After 1 channel\n";

  cru.startHeartbeatFrame(12345, 456);

  std::cout << ">>> After 1 channel + startHB\n";
  cru.printStatus(1);
  std::cout << "<<< After 1 channel + startHB\n";

  // elinkId = 3;
  //
  // cru.addChannelChargeSum(solarId, elinkId, ts, 3, 13);
  // cru.addChannelChargeSum(solarId, elinkId, ts, 13, 133);
  // cru.addChannelChargeSum(solarId, elinkId, ts, 23, 163);
  //
  // elinkId = 2;
  //
  // cru.addChannelChargeSum(solarId, elinkId, ts, 0, 10);
  // cru.addChannelChargeSum(solarId, elinkId, ts, 1, 20);
  // cru.addChannelChargeSum(solarId, elinkId, ts, 2, 30);
  // cru.addChannelChargeSum(solarId, elinkId, ts, 3, 40);
  //
  // elinkId = 10;
  //
  // cru.addChannelChargeSum(solarId, elinkId, ts, 22, 420);
  // cru.addChannelChargeSum(solarId, elinkId, ts, 23, 430);
  // cru.addChannelChargeSum(solarId, elinkId, ts, 24, 440);
  // cru.addChannelChargeSum(solarId, elinkId, ts, 25, 450);
  // cru.addChannelChargeSum(solarId, elinkId, ts, 26, 460);
  //
  // cru.addChannelChargeSum(solarId, elinkId, ts, 12, 420);
  //
  std::vector<uint32_t> buffer;
  size_t initialSize{13};
  buffer.assign(initialSize, 0);
  size_t n = cru.moveToBuffer(buffer);
  BOOST_CHECK_EQUAL(buffer.size(), initialSize + n);
  BOOST_CHECK_EQUAL(n, 0);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
