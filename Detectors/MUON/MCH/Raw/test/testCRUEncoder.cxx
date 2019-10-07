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
using namespace o2::mch::raw;

void dumpBuffer(gsl::span<uint32_t> buffer)
{
  // dump a buffer, assuming it starts with a RDH

  int i{0};
  for (auto& w : buffer) {
    std::cout << fmt::format("{:08X} ", w);
    if ((i + 1) % 4 == 0) {
      std::cout << "\n";
    }
    ++i;
  }
  RAWDataHeader rdh;
  int index{0};

  while (index < buffer.size()) {
    memcpy(&rdh, &buffer[0] + index, sizeof(rdh));
    std::cout << "----- index " << index << "\n";
    std::cout << rdh << "\n";
    if (rdh.memorySize == 0) {
      std::cout << "OUPS\n";
      throw;
    }
    index += rdh.memorySize / 4;
  }
}

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
  srand(time(nullptr));

  CRUEncoder cru(0);

  uint32_t bx(0);
  uint8_t solarId(0);
  uint8_t elinkId(0);
  uint16_t ts(0);

  cru.startHeartbeatFrame(12345, 678);

  cru.addChannelChargeSum(solarId, elinkId, ts, 0, 10);

  cru.startHeartbeatFrame(12345, 910);

  solarId = 1;
  elinkId = 2;
  cru.addChannelChargeSum(solarId, elinkId, ts, 0, 10);
  solarId = 2;
  elinkId = 3;
  cru.addChannelChargeSum(solarId, elinkId, ts, 0, 10);

  solarId = 12;
  elinkId = 3;

  cru.addChannelChargeSum(solarId, elinkId, ts, 3, 13);
  cru.addChannelChargeSum(solarId, elinkId, ts, 13, 133);
  cru.addChannelChargeSum(solarId, elinkId, ts, 23, 163);

  elinkId = 2;

  cru.addChannelChargeSum(solarId, elinkId, ts, 0, 10);
  cru.addChannelChargeSum(solarId, elinkId, ts, 1, 20);
  cru.addChannelChargeSum(solarId, elinkId, ts, 2, 30);
  cru.addChannelChargeSum(solarId, elinkId, ts, 3, 40);

  elinkId = 10;

  cru.addChannelChargeSum(solarId, elinkId, ts, 22, 420);
  cru.addChannelChargeSum(solarId, elinkId, ts, 23, 430);
  cru.addChannelChargeSum(solarId, elinkId, ts, 24, 440);
  cru.addChannelChargeSum(solarId, elinkId, ts, 25, 450);
  cru.addChannelChargeSum(solarId, elinkId, ts, 26, 460);
  cru.addChannelChargeSum(solarId, elinkId, ts, 12, 420);

  std::cout << "\n\nmoveToBuffer\n\n";
  std::vector<uint32_t> buffer;
  size_t initialSize{13};
  buffer.assign(initialSize, 0);
  size_t n = cru.moveToBuffer(buffer);
  dumpBuffer(gsl::span<uint32_t>(&buffer[initialSize], &buffer[buffer.size() - 1]));
  BOOST_CHECK_EQUAL(buffer.size(), initialSize + n);
  size_t expectedSize = 3 * estimateHBSize(1, 1) + estimateHBSize(1, 6);

  BOOST_CHECK_EQUAL(n, expectedSize);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
