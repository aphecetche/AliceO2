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
///
/// In those tests we are mainly concerned about testinng
/// whether the payloads are actually properly simulated.
///
/// The (RDH,payload) blocks we are generating are _not_
/// as in real life (e.g. not paginated) as we're using a
/// pageSize=0 as 2nd parameter of the createEncoder function
///
#define BOOST_TEST_MODULE Test MCHRaw Encoder
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include "MCHRawEncoder/Encoder.h"
#include "MCHRawEncoder/DataBlock.h"
#include "MCHRawCommon/DataFormats.h"
#include <fmt/printf.h>
#include "DumpBuffer.h"
#include <boost/mpl/list.hpp>

using namespace o2::mch::raw;

template <typename FORMAT>
std::unique_ptr<Encoder> defaultEncoder()
{
  return createEncoder<FORMAT, SampleMode, true>();
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
  auto encoder = createEncoder<T, SampleMode, false>();
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
  // forEachDataBlock(buffer, [](DataBlock b) {
  //   std::cout << b.header << "\n";
  //   impl::dumpBuffer(b.payload, std::cout);
  // });
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
    auto encoder = createEncoder<FORMAT, ChargeSumMode, true>();

    return fillChargeSum(*(encoder.get()));
  }
};

int estimateUserLogicSize(int nofDS, int maxNofChPerDS)
{
  size_t headerSize = 2; // equivalent to 2 64-bits words
  // one 64-bits header and one 64-bits data per channel
  // plus one sync per DS
  // (assuming data = just one sample)
  size_t ndata = (maxNofChPerDS * 2) + nofDS;
  return 8 * (ndata + headerSize); // size in bytes
}

int estimateBareSize(int nofDS, int maxNofChPerGBT)
{
  size_t headerSize = 2; // equivalent to 2 64-bits words
  size_t nbits = nofDS * 50 + maxNofChPerGBT * 90;
  size_t n128bitwords = nbits / 2;
  size_t n64bitwords = n128bitwords * 2;
  return 8 * (n64bitwords + headerSize); // size in bytes
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

BOOST_AUTO_TEST_CASE_TEMPLATE(CheckNumberOfPayloadHeaders, T, testTypes)
{
  auto buffer = CruBufferCreator<T, ChargeSumMode>::makeBuffer();
  int nheaders = o2::mch::raw::countHeaders(buffer);
  BOOST_CHECK_EQUAL(nheaders, 4);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(CheckSize, T, testTypes)
{
  auto buffer = CruBufferCreator<T, ChargeSumMode>::makeBuffer();
  size_t expectedSize = estimateSize<T>();
  BOOST_CHECK_EQUAL(buffer.size(), expectedSize);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
