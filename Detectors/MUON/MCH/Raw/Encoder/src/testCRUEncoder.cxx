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
#include "MCHRawEncoder/Encoder.h"
#include "MCHRawCommon/RDHManip.h"
#include "MCHRawCommon/DataFormats.h"
#include "Headers/RAWDataHeader.h"
#include <fmt/printf.h>

using namespace o2::mch::raw;
using namespace o2::mch::raw::dataformat;

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(cruencoder)

BOOST_AUTO_TEST_CASE(StartHBFrameBunchCrossingMustBe12Bits)
{
  auto cru = createCRUEncoder<Bare, SampleMode>(0);
  BOOST_CHECK_THROW(cru->startHeartbeatFrame(0, 1 << 12), std::invalid_argument);
  BOOST_CHECK_NO_THROW(cru->startHeartbeatFrame(0, 0xFFF));
}

BOOST_AUTO_TEST_CASE(EmptyEncoderHasEmptyBufferIfPhaseIsZero)
{
  srand(time(nullptr));
  auto cru = createCRUEncoderNoPhase<Bare, SampleMode>(0);
  cru->startHeartbeatFrame(12345, 123);
  std::vector<uint8_t> buffer;
  cru->moveToBuffer(buffer);
  BOOST_CHECK_EQUAL(buffer.size(), 0);
}

BOOST_AUTO_TEST_CASE(EmptyEncodeIsNotNecessarilyEmptyDependingOnPhase)
{
  srand(time(nullptr));
  auto cru = createCRUEncoder<Bare, SampleMode>(0);
  cru->startHeartbeatFrame(12345, 123);
  std::vector<uint8_t> buffer;
  cru->moveToBuffer(buffer);
  BOOST_CHECK_GE(buffer.size(), 0);
}

BOOST_AUTO_TEST_CASE(MultipleOrbitsWithNoDataIsAnEmptyBufferIfPhaseIsZero)
{
  srand(time(nullptr));
  auto cru = createCRUEncoderNoPhase<Bare, SampleMode>(0);
  cru->startHeartbeatFrame(12345, 123);
  cru->startHeartbeatFrame(12345, 125);
  cru->startHeartbeatFrame(12345, 312);
  std::vector<uint8_t> buffer;
  cru->moveToBuffer(buffer);
  BOOST_CHECK_EQUAL(buffer.size(), 0);
}

std::vector<uint8_t> createCRUBuffer(int cruId)
{
  auto cru = createCRUEncoderNoPhase<Bare, ChargeSumMode>(cruId);

  uint32_t bx(0);
  uint8_t solarId(0);
  uint8_t elinkId(0);
  uint16_t ts(0);

  cru->startHeartbeatFrame(12345, 678);

  cru->addChannelData(solarId, elinkId, 0, {SampaCluster(ts, 10)});

  cru->startHeartbeatFrame(12345, 910);

  solarId = 1;
  elinkId = 2;
  cru->addChannelData(solarId, elinkId, 0, {SampaCluster(ts, 10)});
  solarId = 2;
  elinkId = 3;
  cru->addChannelData(solarId, elinkId, 0, {SampaCluster(ts, 10)});

  solarId = 12;
  elinkId = 3;

  cru->addChannelData(solarId, elinkId, 3, {SampaCluster(ts, 13)});
  cru->addChannelData(solarId, elinkId, 13, {SampaCluster(ts, 133)});
  cru->addChannelData(solarId, elinkId, 23, {SampaCluster(ts, 163)});

  elinkId = 2;

  cru->addChannelData(solarId, elinkId, 0, {SampaCluster(ts, 10)});
  cru->addChannelData(solarId, elinkId, 1, {SampaCluster(ts, 20)});
  cru->addChannelData(solarId, elinkId, 2, {SampaCluster(ts, 30)});
  cru->addChannelData(solarId, elinkId, 3, {SampaCluster(ts, 40)});

  elinkId = 10;

  cru->addChannelData(solarId, elinkId, 22, {SampaCluster(ts, 420)});
  cru->addChannelData(solarId, elinkId, 23, {SampaCluster(ts, 430)});
  cru->addChannelData(solarId, elinkId, 24, {SampaCluster(ts, 440)});
  cru->addChannelData(solarId, elinkId, 25, {SampaCluster(ts, 450)});
  cru->addChannelData(solarId, elinkId, 26, {SampaCluster(ts, 460)});
  cru->addChannelData(solarId, elinkId, 12, {SampaCluster(ts, 420)});

  std::vector<uint8_t> buffer;
  cru->moveToBuffer(buffer);
  int i{0};
  for (auto v : buffer) {
    fmt::printf("0x%02X, ", v);
    if (++i % 12 == 0) {
      fmt::printf("\n");
    }
  }
  return buffer;
}

int estimateHBSize(int nofGbts, int maxNofChPerGbt)
{
  size_t rdhSize = 16; // 16 32-bits words
  size_t nbits = (50 + maxNofChPerGbt * 90);
  size_t n128bitwords = nbits / 2;
  size_t n32bitwords = n128bitwords * 4;
  return 4 * (n32bitwords + rdhSize * nofGbts); // size in bytes
}

BOOST_AUTO_TEST_CASE(CheckNumberOfRDHs)
{
  auto buffer = createCRUBuffer(0);
  int nrdh = o2::mch::raw::countRDHs<o2::header::RAWDataHeaderV4>(buffer);
  size_t expectedSize = 3 * estimateHBSize(1, 1) + estimateHBSize(1, 6);
  BOOST_CHECK_EQUAL(buffer.size(), expectedSize);
  BOOST_CHECK_EQUAL(nrdh, 4);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
