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
#include "TestBuffers.h"

using namespace o2::mch::raw;

std::optional<uint16_t> dummyMapper(uint16_t solarId)
{
  if (solarId == 728) {
    return 0;
  }
  if (solarId == 361) {
    return 1;
  }
  if (solarId == 448) {
    return 2;
  }
  return std::nullopt;
}
std::unique_ptr<Encoder> defaultEncoder()
{
  return createEncoder<BareFormat, SampleMode, o2::header::RAWDataHeaderV4, true>(dummyMapper);
}
BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(encoder)

BOOST_AUTO_TEST_CASE(StartHBFrameBunchCrossingMustBe12Bits)
{
  auto encoder = defaultEncoder();
  BOOST_CHECK_THROW(encoder->startHeartbeatFrame(0, 1 << 12), std::invalid_argument);
  BOOST_CHECK_NO_THROW(encoder->startHeartbeatFrame(0, 0xFFF));
}

BOOST_AUTO_TEST_CASE(EmptyEncoderHasEmptyBufferIfPhaseIsZero)
{
  srand(time(nullptr));
  auto encoder = defaultEncoder();
  encoder->startHeartbeatFrame(12345, 123);
  std::vector<uint8_t> buffer;
  encoder->moveToBuffer(buffer);
  BOOST_CHECK_EQUAL(buffer.size(), 0);
}

BOOST_AUTO_TEST_CASE(EmptyEncodeIsNotNecessarilyEmptyDependingOnPhase)
{
  srand(time(nullptr));
  auto encoder = createEncoder<BareFormat, SampleMode, o2::header::RAWDataHeaderV4, false>(dummyMapper);
  encoder->startHeartbeatFrame(12345, 123);
  std::vector<uint8_t> buffer;
  encoder->moveToBuffer(buffer);
  BOOST_CHECK_GE(buffer.size(), 0);
}

BOOST_AUTO_TEST_CASE(MultipleOrbitsWithNoDataIsAnEmptyBufferIfPhaseIsZero)
{
  srand(time(nullptr));
  auto encoder = defaultEncoder();
  encoder->startHeartbeatFrame(12345, 123);
  encoder->startHeartbeatFrame(12345, 125);
  encoder->startHeartbeatFrame(12345, 312);
  std::vector<uint8_t> buffer;
  encoder->moveToBuffer(buffer);
  BOOST_CHECK_EQUAL(buffer.size(), 0);
}

std::vector<uint8_t> createCRUBuffer()
{

  auto encoder = createEncoder<BareFormat, ChargeSumMode, o2::header::RAWDataHeaderV4, true>(dummyMapper);

  uint16_t ts(0);

  encoder->startHeartbeatFrame(12345, 678);

  encoder->addChannelData(DsElecId{728, 1, 2}, 0, {SampaCluster(ts, 10)});

  encoder->startHeartbeatFrame(12345, 910);

  encoder->addChannelData(DsElecId{728, 1, 2}, 0, {SampaCluster(ts, 10)});
  encoder->addChannelData(DsElecId{728, 1, 2}, 0, {SampaCluster(ts, 10)});

  encoder->addChannelData(DsElecId{728, 1, 0}, 3, {SampaCluster(ts, 13)});
  encoder->addChannelData(DsElecId{728, 1, 0}, 13, {SampaCluster(ts, 133)});
  encoder->addChannelData(DsElecId{728, 1, 0}, 23, {SampaCluster(ts, 163)});

  encoder->addChannelData(DsElecId{361, 0, 4}, 0, {SampaCluster(ts, 10)});
  encoder->addChannelData(DsElecId{361, 0, 4}, 1, {SampaCluster(ts, 20)});
  encoder->addChannelData(DsElecId{361, 0, 4}, 2, {SampaCluster(ts, 30)});
  encoder->addChannelData(DsElecId{361, 0, 4}, 3, {SampaCluster(ts, 40)});

  encoder->addChannelData(DsElecId{448, 6, 2}, 22, {SampaCluster(ts, 420)});
  encoder->addChannelData(DsElecId{448, 6, 2}, 23, {SampaCluster(ts, 430)});
  encoder->addChannelData(DsElecId{448, 6, 2}, 24, {SampaCluster(ts, 440)});
  encoder->addChannelData(DsElecId{448, 6, 2}, 25, {SampaCluster(ts, 450)});
  encoder->addChannelData(DsElecId{448, 6, 2}, 26, {SampaCluster(ts, 460)});
  encoder->addChannelData(DsElecId{448, 6, 2}, 12, {SampaCluster(ts, 420)});

  std::vector<uint8_t> buffer;
  encoder->moveToBuffer(buffer);
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
  auto buffer = createCRUBuffer();
  int nrdh = o2::mch::raw::countRDHs<o2::header::RAWDataHeaderV4>(buffer);
  size_t expectedSize = 3 * estimateHBSize(1, 1) + estimateHBSize(1, 6);
  BOOST_CHECK_EQUAL(buffer.size(), expectedSize);
  BOOST_CHECK_EQUAL(nrdh, 4);
}

BOOST_AUTO_TEST_CASE(GenerateBarePedestalBuffer)
{
  auto buffer = impl::createPedestalBuffer<BareFormat, SampleMode, o2::header::RAWDataHeaderV4>(0);
  BOOST_CHECK_EQUAL(buffer.size(), 57504);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
