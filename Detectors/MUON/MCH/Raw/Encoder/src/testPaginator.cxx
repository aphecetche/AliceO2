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

#define BOOST_TEST_MODULE Test MCHRaw Paginator
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include "DumpBuffer.h"
#include "Headers/RAWDataHeader.h"
#include "MCHRawCommon/DataFormats.h"
#include "MCHRawCommon/RDHManip.h"
#include "MCHRawEncoder/Encoder.h"
#include "MCHRawEncoder/Paginator.h"
#include "TestBuffers.h"
#include <boost/test/data/test_case.hpp>
#include <fmt/printf.h>
#include <fstream>
#include <iostream>

using namespace o2::mch::raw;
using namespace o2::header;

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(encoder)

BOOST_AUTO_TEST_CASE(TestPadding)
{
  std::vector<uint8_t> data;

  data.emplace_back(0x22);
  data.emplace_back(0x22);
  data.emplace_back(0x22);
  data.emplace_back(0x22);
  data.emplace_back(0x44);
  data.emplace_back(0x44);
  data.emplace_back(0x44);
  data.emplace_back(0x44);
  data.emplace_back(0x66);
  data.emplace_back(0x66);
  data.emplace_back(0x66);
  data.emplace_back(0x66);
  data.emplace_back(0x88);
  data.emplace_back(0x88);
  data.emplace_back(0x88);
  data.emplace_back(0x88);

  auto buffer = impl::createTestBuffer<RAWDataHeaderV4>(data);

  std::vector<uint8_t> pages;
  size_t pageSize = 128;
  uint8_t paddingByte = 0x78;
  paginateBuffer<RAWDataHeaderV4>(buffer, pages, pageSize, paddingByte);

  BOOST_CHECK_EQUAL(pages.size(), pageSize);
  for (int i = sizeof(RAWDataHeaderV4) + data.size(); i < pageSize; i++) {
    BOOST_CHECK_EQUAL(pages[i], paddingByte);
  }
}

BOOST_DATA_TEST_CASE(TestSplit,
                     boost::unit_test::data::make({12, 48, 400, 16320}) *
                       boost::unit_test::data::make({128, 512, 8192}),
                     ndata, pageSize)
{
  constexpr uint8_t paddingWord = 0x55;

  std::vector<uint8_t> data;
  for (int i = 0; i < ndata * 4; i++) {
    data.emplace_back((i + 1) % 256);
  }

  auto buffer = impl::createTestBuffer<RAWDataHeaderV4>(data);

  std::vector<uint8_t> pages;
  int expected = std::ceil(1.0 * data.size() / (pageSize - sizeof(RAWDataHeader)));
  paginateBuffer<RAWDataHeaderV4>(buffer, pages, pageSize, paddingWord);
  int nrdhs = countRDHs<RAWDataHeaderV4>(pages);
  BOOST_CHECK_EQUAL(nrdhs, expected);

  // ensure all words ended up where we expect them
  bool ok{true};
  int n{0};
  for (int i = 0; i < expected; i++) {
    int dataPos = sizeof(RAWDataHeader) + i * pageSize;
    for (int j = 0; j < pageSize - sizeof(RAWDataHeaderV4); j++) {
      ++n;
      if (n >= data.size()) {
        break;
      }
      if (pages[dataPos + j] != (n % 256)) {
        ok = false;
      }
    }
  }
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(n, data.size());
}

BOOST_AUTO_TEST_CASE(GenerateBareFile)
{
  std::ofstream out("test.bare.raw", std::ios::binary);
  auto buffer = impl::createPedestalBuffer<BareFormat, SampleMode, o2::header::RAWDataHeaderV4>(0);
  std::vector<uint8_t> pages;
  paginateBuffer<RAWDataHeaderV4>(buffer, pages, 8192, 0x44);
  out.write(reinterpret_cast<char*>(&pages[0]), pages.size());
  out.close();
}

BOOST_AUTO_TEST_CASE(GenerateUserLogicFile)
{
  std::ofstream out("test.raw", std::ios::binary);
  auto buffer = impl::createPedestalBuffer<UserLogicFormat, SampleMode, o2::header::RAWDataHeaderV4>(0);
  impl::dumpBuffer(buffer);
  std::vector<uint8_t> pages;
  paginateBuffer<RAWDataHeaderV4>(buffer, pages, 8192, 0x44);
  out.write(reinterpret_cast<char*>(&pages[0]), pages.size());
  out.close();
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
