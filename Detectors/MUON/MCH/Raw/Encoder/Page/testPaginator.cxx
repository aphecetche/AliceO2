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
#include "MCHRawEncoder/DataBlock.h"
#include "Paginator.h"
#include <boost/mpl/list.hpp>
#include <boost/test/data/test_case.hpp>
#include <fmt/printf.h>
#include <fstream>
#include <iostream>
#include <cstddef>

using namespace o2::mch::raw;
using namespace o2::header;

typedef boost::mpl::list<o2::header::RAWDataHeaderV4> testTypes;

extern std::ostream& operator<<(std::ostream&, const o2::header::RAWDataHeaderV4&);

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(encoder)

std::vector<std::byte> createDataBlock(gsl::span<std::byte> payload)
{
  std::vector<std::byte> outBuffer;
  uint16_t solarId{968}; // must be a valid solar identifier
  DataBlockHeader header{0, 0, solarId, static_cast<uint16_t>(payload.size())};
  appendDataBlockHeader(outBuffer, header);
  outBuffer.insert(outBuffer.end(), payload.begin(), payload.end());
  return outBuffer;
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TestPadding, RDH, testTypes)
{
  std::vector<std::byte> data;

  data.emplace_back(std::byte{0x22});
  data.emplace_back(std::byte{0x22});
  data.emplace_back(std::byte{0x22});
  data.emplace_back(std::byte{0x22});
  data.emplace_back(std::byte{0x44});
  data.emplace_back(std::byte{0x44});
  data.emplace_back(std::byte{0x44});
  data.emplace_back(std::byte{0x44});
  data.emplace_back(std::byte{0x66});
  data.emplace_back(std::byte{0x66});
  data.emplace_back(std::byte{0x66});
  data.emplace_back(std::byte{0x66});
  data.emplace_back(std::byte{0x88});
  data.emplace_back(std::byte{0x88});
  data.emplace_back(std::byte{0x88});
  data.emplace_back(std::byte{0x88});

  std::vector<std::byte> pages;

  size_t pageSize = 128;
  std::byte paddingByte = std::byte{0x78};
  auto buffer = createDataBlock(data);
  paginateBuffer<RDH>(buffer, pages, pageSize, paddingByte);

  size_t expected = pageSize * 2; // +1 STOP page per actual page
  BOOST_CHECK_EQUAL(pages.size(), expected);
  for (int i = sizeof(RDH) + data.size(); i < pageSize; i++) {
    BOOST_CHECK(pages[i] == paddingByte);
  }
}

BOOST_DATA_TEST_CASE(TestSplit,
                     boost::unit_test::data::make({12, 48, 400, 16320}) *
                       boost::unit_test::data::make({0, 128, 512, 8192}),
                     ndata, pageSize)
{
  constexpr std::byte paddingByte = std::byte{0x55};

  std::vector<std::byte> data;
  for (int i = 0; i < ndata * 4; i++) {
    data.emplace_back(std::byte{static_cast<uint8_t>((i + 1) % 256)});
  }

  std::vector<std::byte> pages;
  int npages = 1;
  if (pageSize > 0) {
    npages = std::ceil(1.0 * data.size() / (pageSize - sizeof(RAWDataHeader)));
  }
  int nstop = 1;
  int nexpected = npages + nstop;
  auto buffer = createDataBlock(data);
  int nsplit = paginateBuffer<RAWDataHeaderV4>(buffer, pages, pageSize, paddingByte);
  int nrdhs = countRDHs<RAWDataHeaderV4>(pages);
  BOOST_CHECK_EQUAL(nrdhs, nexpected);

  // ensure all words ended up where we expect them
  bool ok{true};
  int n{0};
  for (int i = 0; i < npages; i++) {
    int dataPos = sizeof(RAWDataHeaderV4) + i * pageSize;
    for (int j = 0; j < pageSize - sizeof(RAWDataHeaderV4); j++) {
      ++n;
      if (n >= data.size()) {
        break;
      }
      if (std::to_integer<int>(pages[dataPos + j]) != (n % 256)) {
        ok = false;
      }
    }
  }
  BOOST_CHECK_EQUAL(n, data.size());
  BOOST_CHECK_EQUAL(ok, true);
}

BOOST_AUTO_TEST_CASE(CannotPaginateIfPageSizeIsSmallerThanRDHSize)
{
  std::vector<std::byte> buffer;
  std::vector<std::byte> pages;
  BOOST_CHECK_THROW(paginateBuffer<RAWDataHeaderV4>(buffer, pages, 24, std::byte{0xFF}),
                    std::invalid_argument);
  BOOST_CHECK_NO_THROW(paginateBuffer<RAWDataHeaderV4>(buffer, pages, sizeof(RAWDataHeaderV4) + 8, std ::byte{0xFF}));
}

BOOST_DATA_TEST_CASE(PaginateEmptyPayloads,
                     boost::unit_test::data::make({72, 8192}),
                     pageSize)
{
  constexpr std::byte paddingByte = std::byte{0x66};
  std::vector<std::byte> buffer;
  //std::array<uint16_t, 5> feeIds = {631, 729, 424, 12, 42};
  std::array<uint16_t, 5> feeIds = {144, 72, 448, 728, 968};
  for (auto feeId : feeIds) {
    DataBlockHeader header{12345, 678, feeId, 0};
    appendDataBlockHeader(buffer, header);
  }
  BOOST_REQUIRE_EQUAL(buffer.size(), feeIds.size() * sizeof(DataBlockHeader));

  std::vector<std::byte> pages;
  paginateBuffer<RAWDataHeaderV4>(buffer, pages, pageSize, paddingByte);
  int expectedNofRDHs(feeIds.size() * 2); // 1 page + 1 stop page per link

  BOOST_REQUIRE_EQUAL(pages.size(), expectedNofRDHs * pageSize);

  int nrdhs = countRDHs<RAWDataHeaderV4>(pages);
  BOOST_CHECK_EQUAL(nrdhs, expectedNofRDHs);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
