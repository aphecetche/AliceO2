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
#include "MCHRaw/RAWDataHeader.h"
#include "common.h"

using namespace o2::mch::raw;

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(cruencoder)

BOOST_AUTO_TEST_CASE(StartHBFrameBunchCrossingMustBe12Bits)
{
  CRUEncoder cru(0);
  BOOST_CHECK_THROW(cru.startHeartbeatFrame(0, 1 << 12), std::invalid_argument);
  BOOST_CHECK_NO_THROW(cru.startHeartbeatFrame(0, 0xFFF));
}

BOOST_AUTO_TEST_CASE(EmptyEncoderHasEmptyBufferIfPhaseIsZero)
{
  srand(time(nullptr));
  GBTEncoder::forceNoPhase = true;
  CRUEncoder cru(0);
  cru.startHeartbeatFrame(12345, 123);
  std::vector<uint32_t> buffer;
  cru.moveToBuffer(buffer);
  BOOST_CHECK_EQUAL(buffer.size(), 0);
}

BOOST_AUTO_TEST_CASE(EmptyEncodeIsNotNecessarilyEmptyDependingOnPhase)
{
  srand(time(nullptr));
  GBTEncoder::forceNoPhase = false;
  CRUEncoder cru(0);
  cru.startHeartbeatFrame(12345, 123);
  std::vector<uint32_t> buffer;
  cru.moveToBuffer(buffer);
  BOOST_CHECK_GE(buffer.size(), 0);
}

BOOST_AUTO_TEST_CASE(MultipleOrbitsWithNoDataIsAnEmptyBufferIfPhaseIsZero)
{
  srand(time(nullptr));
  GBTEncoder::forceNoPhase = true;
  CRUEncoder cru(0);
  cru.startHeartbeatFrame(12345, 123);
  cru.startHeartbeatFrame(12345, 125);
  cru.startHeartbeatFrame(12345, 312);
  std::vector<uint32_t> buffer;
  cru.moveToBuffer(buffer);
  BOOST_CHECK_EQUAL(buffer.size(), 0);
}

int estimateHBSize(int nofGbts, int maxNofChPerGbt)
{
  size_t rdhSize = 16; // 16 32-bits words
  size_t nbits = (50 + maxNofChPerGbt * 90);
  size_t n128bitwords = nbits / 2;
  size_t n32bitwords = n128bitwords * 4;
  return n32bitwords + rdhSize * nofGbts;
}

BOOST_AUTO_TEST_CASE(CheckNumberOfRDHs)
{
  auto buffer = o2::mch::raw::test::createCRUBuffer();
  int nrdh = o2::mch::raw::test::countRDHs(buffer);
  size_t expectedSize = 3 * estimateHBSize(1, 1) + estimateHBSize(1, 6);
  BOOST_CHECK_EQUAL(buffer.size(), expectedSize);
  BOOST_CHECK_EQUAL(nrdh, 4);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
