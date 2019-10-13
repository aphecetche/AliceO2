// Copyright CERN and copyright holders of ALICE O2. This software is
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

#define BOOST_TEST_MODULE Test MCHRaw Encoder
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <iostream>
#include "MCHRaw/RAWDataHeader.h"
#include <fstream>
#include <fmt/printf.h>
#include "MCHRaw/Encoder.h"
#include "MCHRaw/CRUEncoder.h"
#include <boost/test/data/test_case.hpp>

using namespace o2::mch::raw;
using RDH = o2::mch::raw::RAWDataHeader;

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(encoder)

std::vector<uint32_t> createTestBuffer(gsl::span<uint32_t> data)
{
  assert(data.size() % 4 == 0);
  std::vector<uint32_t> buffer;
  auto payloadSize = data.size() * sizeof(uint32_t);
  if (payloadSize > (1 << 16) - sizeof(RAWDataHeader)) {
    throw std::logic_error(fmt::format("cannot generate a buffer with a payload above {} (tried {})", 0xFFFF - sizeof(RAWDataHeader), payloadSize));
  }
  auto rdh = createRDH(0, 0, 1234, 567, payloadSize);
  appendRDH(buffer, rdh);
  std::copy(data.begin(), data.end(), std::back_inserter(buffer));
  return buffer;
}

std::vector<uint32_t> createPedestalBuffer(int elinkId)
{
  uint32_t bx(0);
  uint8_t solarId(0);
  uint16_t ts(0);
  uint8_t cruId(0);

  GBTEncoder::forceNoPhase = true;
  CRUEncoder cru(cruId);

  uint32_t orbit{42};
  const int N{1};

  for (int i = 0; i < N; i++) {
    cru.startHeartbeatFrame(orbit, bx + i);
    cru.addChannelData(solarId, elinkId, i, {SampaCluster(ts, i)});
  }
  std::vector<uint32_t> buffer;
  cru.moveToBuffer(buffer);
  return buffer;
}

BOOST_AUTO_TEST_CASE(TestPadding)
{
  std::vector<uint32_t> data;

  data.emplace_back(0x22222222);
  data.emplace_back(0x44444444);
  data.emplace_back(0x66666666);
  data.emplace_back(0x88888888);

  auto buffer = createTestBuffer(data);

  std::vector<uint32_t> pages;
  size_t pageSize = 128;
  uint32_t paddingWord = 0x12345678;
  paginateBuffer(buffer, pages, pageSize, paddingWord);

  BOOST_CHECK_EQUAL(pages.size(), pageSize / 4);
  for (int i = sizeof(RAWDataHeader) / 4 + data.size(); i < pageSize / 4; i++) {
    BOOST_CHECK_EQUAL(pages[i], paddingWord);
  }
}

BOOST_DATA_TEST_CASE(TestSplit,
                     boost::unit_test::data::make({12, 48, 400, 16320}) *
                       boost::unit_test::data::make({128, 512, 8192}),
                     ndata, pageSize)
{
  constexpr uint32_t paddingWord = 0x12345678;

  std::vector<uint32_t> data;
  for (int i = 0; i < ndata; i++) {
    data.emplace_back(i + 1);
  }

  auto buffer = createTestBuffer(data);

  std::vector<uint32_t> pages;
  int expected = std::ceil(1.0 * (data.size() * 4) / (pageSize - sizeof(RAWDataHeader)));
  paginateBuffer(buffer, pages, pageSize, paddingWord);

  int nrdhs = o2::mch::raw::countRDHs(pages);
  BOOST_CHECK_EQUAL(nrdhs, expected);

  // ensure all words ended up where we expect them
  bool ok{true};
  int n{0};
  for (int i = 0; i < expected; i++) {
    int dataPos = sizeof(RAWDataHeader) / 4 + i * pageSize / 4;
    for (int j = 0; j < pageSize / 4 - sizeof(RAWDataHeader) / 4; j++) {
      ++n;
      if (n >= data.size()) {
        break;
      }
      if (pages[dataPos + j] != n) {
        ok = false;
      }
    }
  }
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(n, data.size());
}

BOOST_AUTO_TEST_CASE(GenerateFile)
{
  std::ofstream out("test.raw", std::ios::binary);
  auto buffer = createPedestalBuffer(0);
  std::cout << "buffer.size=" << buffer.size() << "\n";

  dumpBuffer(buffer);

  // for (int i = 0; i < 3; i++) {
  //   std::vector<uint32_t> pages;
  //   paginateBuffer(buffer, pages, 8192, 0x0);
  //   dumpBuffer(pages);
  //   out.write(reinterpret_cast<char*>(&buffer[0]), buffer.size() * sizeof(uint32_t));
  // }
  out.close();
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
