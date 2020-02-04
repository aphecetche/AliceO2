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
///
#define BOOST_TEST_MODULE Test MCHRaw Normalizer
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include "MCHRawEncoder/Encoder.h"
#include "MCHRawEncoder/Normalizer.h"
#include "MCHRawEncoder/DataBlock.h"
#include "MCHRawCommon/DataFormats.h"
#include "MCHRawCommon/RDHManip.h"
#include <fmt/printf.h>
#include "DumpBuffer.h"
#include <boost/mpl/list.hpp>
#include "CruBufferCreator.h"
#include "Paginator.h"
#include "LinkRange.h"

using namespace o2::mch::raw;

typedef boost::mpl::list<o2::header::RAWDataHeaderV4> testTypes;

struct Common {
  constexpr static size_t pageSize = 128;
  constexpr static uint8_t paddingByte = 0x44;
  std::vector<uint8_t> buffer;
  std::vector<o2::InteractionTimeRecord> interactions;
  struct TestPoint {
    int pageSize, expectedNofRDHs;
  };
  std::vector<TestPoint> testPoints;
  Common() : buffer(test::CruBufferCreator<BareFormat, ChargeSumMode>::makeBuffer()),
             interactions{o2::InteractionTimeRecord{o2::InteractionRecord{0, 0}, 0}},
             testPoints{
               //{8192, 8},
               {1024, 18},
               // {512, 31},
               // {128, 183}
             }
  {
    forEachDataBlock(buffer, [](const DataBlock& block) {
      std::cout << block.header << "\n";
    });
  }
} F;

uint64_t orbitbc(uint32_t orbit, uint16_t bc)
{
  return static_cast<uint64_t>(orbit) | ((static_cast<uint64_t>(bc) << 32));
}

template <typename RDH>
bool pageCountsMustBeIncreasingByOne(gsl::span<const uint8_t> pages)
{
  bool ok{true};
  std::map<uint64_t, std::map<int, std::vector<int>>> pageCounts;
  forEachRDH<RDH>(pages, [&pageCounts](const RDH& rdh) {
    (pageCounts[orbitbc(rdhOrbit(rdh), rdhBunchCrossing(rdh))])[rdhFeeId(rdh)].emplace_back(rdhPageCounter(rdh));
  });

  for (auto ob : pageCounts) {
    uint32_t orbit = static_cast<uint32_t>(ob.first & 0xFFFF);
    uint16_t bc = static_cast<uint16_t>((ob.first >> 32) & 0xFFFF);
    std::cout << "OB " << orbit << " " << bc << "\n";
    for (auto p : ob.second) {
      std::cout << "Link " << p.first << " : ";
      for (auto pc : p.second) {
        std::cout << pc << ",";
      }
      for (auto i = 0; i < p.second.size() - 1; i++) {
        if (p.second[i] != p.second[i + 1] - 1) {
          std::cout << "ERROR";
          ok = false;
        }
      }
      std::cout << "\n";
    }
  }
  return ok;
}

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(encoder)

BOOST_AUTO_TEST_CASE_TEMPLATE(CheckNumberOfRDHs, RDH, testTypes)
{
  for (auto s : F.testPoints) {
    std::vector<uint8_t> pages;
    o2::mch::raw::normalizeBuffer<RDH>(F.buffer, pages, F.interactions, s.pageSize, F.paddingByte);
    auto nrdhs = o2::mch::raw::countRDHs<RDH>(pages);
    BOOST_CHECK_EQUAL(nrdhs, s.expectedNofRDHs);
  }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(CheckPageCounter, RDH, testTypes)
{
  for (auto s : F.testPoints) {
    std::vector<uint8_t> pages;
    o2::mch::raw::normalizeBuffer<RDH>(F.buffer, pages, F.interactions, s.pageSize, F.paddingByte);
    bool ok = pageCountsMustBeIncreasingByOne<RDH>(pages);
    BOOST_CHECK_EQUAL(ok, true);
  }
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
