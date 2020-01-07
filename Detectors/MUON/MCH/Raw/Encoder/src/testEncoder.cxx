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
#include "DumpBuffer.h"
#include <boost/mpl/list.hpp>

using namespace o2::mch::raw;

std::optional<uint16_t> dummyMapper(uint16_t solarId)
{
  if (solarId == 728) {
    return 3;
  }
  if (solarId == 361) {
    return 11;
  }
  if (solarId == 448) {
    return 23;
  }
  return std::nullopt;
}

template <typename FORMAT>
std::unique_ptr<Encoder> defaultEncoder()
{
  return createEncoder<FORMAT, SampleMode, o2::header::RAWDataHeaderV4, true>(dummyMapper);
}

typedef boost::mpl::list<BareFormat, UserLogicFormat> testTypes;

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(encoder)

BOOST_AUTO_TEST_CASE_TEMPLATE(StartHBFrameBunchCrossingMustBe12Bits, T, testTypes)
{
  auto encoder = defaultEncoder<T>();
  BOOST_CHECK_THROW(encoder->startHeartbeatFrame(0, 1 << 12), std::invalid_argument);
  BOOST_CHECK_NO_THROW(encoder->startHeartbeatFrame(0, 0xFFF));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(EmptyEncoderHasEmptyBufferIfPhaseIsZero, T, testTypes)
{
  srand(time(nullptr));
  auto encoder = defaultEncoder<T>();
  encoder->startHeartbeatFrame(12345, 123);
  std::vector<uint8_t> buffer;
  encoder->moveToBuffer(buffer);
  BOOST_CHECK_EQUAL(buffer.size(), 0);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(EmptyEncodeIsNotNecessarilyEmptyDependingOnPhase, T, testTypes)
{
  srand(time(nullptr));
  auto encoder = createEncoder<T, SampleMode, o2::header::RAWDataHeaderV4, false>(dummyMapper);
  encoder->startHeartbeatFrame(12345, 123);
  std::vector<uint8_t> buffer;
  encoder->moveToBuffer(buffer);
  BOOST_CHECK_GE(buffer.size(), 0);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(MultipleOrbitsWithNoDataIsAnEmptyBufferIfPhaseIsZero, T, testTypes)
{
  srand(time(nullptr));
  auto encoder = defaultEncoder<T>();
  encoder->startHeartbeatFrame(12345, 123);
  encoder->startHeartbeatFrame(12345, 125);
  encoder->startHeartbeatFrame(12345, 312);
  std::vector<uint8_t> buffer;
  encoder->moveToBuffer(buffer);
  BOOST_CHECK_EQUAL(buffer.size(), 0);
}

std::vector<uint8_t> fillChargeSum(Encoder& encoder)
{
  uint16_t ts(0);

  encoder.startHeartbeatFrame(12345, 678);

  encoder.addChannelData(DsElecId{728, 1, 2}, 0, {SampaCluster(ts, 10)});

  encoder.startHeartbeatFrame(12345, 910);

  encoder.addChannelData(DsElecId{728, 1, 2}, 0, {SampaCluster(ts, 10)});
  encoder.addChannelData(DsElecId{728, 1, 2}, 1, {SampaCluster(ts, 10)});

  encoder.addChannelData(DsElecId{728, 1, 0}, 3, {SampaCluster(ts, 13)});
  encoder.addChannelData(DsElecId{728, 1, 0}, 13, {SampaCluster(ts, 133)});
  encoder.addChannelData(DsElecId{728, 1, 0}, 23, {SampaCluster(ts, 163)});

  encoder.addChannelData(DsElecId{361, 0, 4}, 0, {SampaCluster(ts, 10)});
  encoder.addChannelData(DsElecId{361, 0, 4}, 1, {SampaCluster(ts, 20)});
  encoder.addChannelData(DsElecId{361, 0, 4}, 2, {SampaCluster(ts, 30)});
  encoder.addChannelData(DsElecId{361, 0, 4}, 3, {SampaCluster(ts, 40)});

  encoder.addChannelData(DsElecId{448, 6, 2}, 22, {SampaCluster(ts, 420)});
  encoder.addChannelData(DsElecId{448, 6, 2}, 23, {SampaCluster(ts, 430)});
  encoder.addChannelData(DsElecId{448, 6, 2}, 24, {SampaCluster(ts, 440)});
  encoder.addChannelData(DsElecId{448, 6, 2}, 25, {SampaCluster(ts, 450)});
  encoder.addChannelData(DsElecId{448, 6, 2}, 26, {SampaCluster(ts, 460)});
  encoder.addChannelData(DsElecId{448, 6, 2}, 12, {SampaCluster(ts, 420)});

  std::vector<uint8_t> buffer;
  encoder.moveToBuffer(buffer);
  // int i{0};
  // for (auto v : buffer) {
  //   fmt::printf("0x%02X, ", v);
  //   if (++i % 12 == 0) {
  //     fmt::printf("\n");
  //   }
  // }
  //   o2::mch::raw::impl::dumpBuffer(buffer);
  return buffer;
}

template <typename FORMAT, typename CHARGESUM>
struct CruBufferCreator {

  static std::vector<uint8_t> makeBuffer();
};

template <typename FORMAT>
struct CruBufferCreator<FORMAT, ChargeSumMode> {
  static std::vector<uint8_t> makeBuffer()
  {
    auto encoder = createEncoder<FORMAT, ChargeSumMode, o2::header::RAWDataHeaderV4, true>(dummyMapper);

    return fillChargeSum(*(encoder.get()));
  }
};

int estimateUserLogicSize(int nofDS, int maxNofChPerDS)
{
  size_t rdhSize = 8; // 8 64-bits words
  // one 64-bits header and one 64-bits data per channel
  // plus one sync per DS
  // (assuming data = just one sample)
  size_t ndata = (maxNofChPerDS * 2) + nofDS;
  return 8 * (ndata + rdhSize); // size in bytes
}

int estimateBareSize(int nofDS, int maxNofChPerGBT)
{
  size_t rdhSize = 16; // 16 32-bits words
  size_t nbits = nofDS * 50 + maxNofChPerGBT * 90;
  size_t n128bitwords = nbits / 2;
  size_t n32bitwords = n128bitwords * 4;
  return 4 * (n32bitwords + rdhSize); // size in bytes
}

template <typename FORMAT>
int estimateSize();

template <>
int estimateSize<BareFormat>()
{
  return estimateBareSize(1, 0) +
         estimateBareSize(2, 3) +
         estimateBareSize(1, 4) +
         estimateBareSize(1, 6);
}

template <>
int estimateSize<UserLogicFormat>()
{
  return estimateUserLogicSize(2, 1) +
         estimateUserLogicSize(1, 5) +
         estimateUserLogicSize(1, 4) +
         estimateUserLogicSize(1, 6);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(CheckNumberOfRDHs, T, testTypes)
{
  auto buffer = CruBufferCreator<T, ChargeSumMode>::makeBuffer();
  int nrdh = o2::mch::raw::countRDHs<o2::header::RAWDataHeaderV4>(buffer);
  BOOST_CHECK_EQUAL(nrdh, 4);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(CheckSize, T, testTypes)
{
  auto buffer = CruBufferCreator<T, ChargeSumMode>::makeBuffer();
  size_t expectedSize = estimateSize<T>();
  BOOST_CHECK_EQUAL(buffer.size(), expectedSize);
}

BOOST_AUTO_TEST_CASE(GenerateBarePedestalBuffer)
{
  auto buffer = impl::createPedestalBuffer<BareFormat, SampleMode, o2::header::RAWDataHeaderV4>(0);
  BOOST_CHECK_EQUAL(buffer.size(), 57504);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
