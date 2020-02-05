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
#include "CommonConstants/Triggers.h"
#include "DetectorsRaw/HBFUtils.h"

using namespace o2::mch::raw;

extern std::ostream& operator<<(std::ostream&, const o2::header::RAWDataHeaderV4&);

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
  Common() : buffer(test::CruBufferCreator<BareFormat, ChargeSumMode>::makeBuffer(3)),
             interactions{},
             testPoints{
               {8192, 30},
               {1024, 43},
               {512, 63},
               {128, 282}}
  {

    std::set<o2::InteractionTimeRecord> irs;
    // get the IRs directly from the created buffer
    forEachDataBlockRef(buffer, [&irs](const DataBlockRef& ref) {
      irs.emplace(o2::InteractionRecord{ref.block.header.bc, ref.block.header.orbit}, 0);
    });
    interactions.insert(interactions.end(), irs.begin(), irs.end());
  }
} F;

template <typename RDH>
bool packetCounterMustBeIncreasingByOne(gsl::span<const uint8_t> pages)
{
  // packetCounts is a map (feeId->{packetCounts})
  std::map<uint16_t, std::vector<int>> packetCounts;
  forEachRDH<RDH>(pages, [&packetCounts](const RDH& rdh) {
    packetCounts[rdhFeeId(rdh)].emplace_back(rdhPacketCounter(rdh));
  });
  bool ok{true};
  // check that for each feeId the packetcounter is increasing
  // by one at each RDH.
  for (auto pc : packetCounts) {
    if (pc.second.size() > 1) {
      for (auto i = 0; i < pc.second.size() - 1; i++) {
        if (pc.second[i] != pc.second[i + 1] - 1) {
          ok = false;
        }
      }
    }
  }
  return ok;
}

template <typename RDH>
bool pageCountsMustBeIncreasingByOne(gsl::span<const uint8_t> pages)
{
  // pageCounts is a map (orbc -> (feeId,{pageCounts}))
  // where orb is a combination of orbit and bc
  // {pageCounts} is a vector of page counters
  std::map<uint64_t, std::map<int, std::vector<int>>> pageCounts;
  forEachRDH<RDH>(pages, [&pageCounts](const RDH& rdh) {
    uint64_t orbc =
      static_cast<uint64_t>(rdhOrbit(rdh)) |
      ((static_cast<uint64_t>(rdhBunchCrossing(rdh)) << 32));
    (pageCounts[orbc])[rdhFeeId(rdh)].emplace_back(rdhPageCounter(rdh));
  });

  bool ok{true};

  // check that for each fee and each (orbit,bc)
  // the vector of {pageCounts} goes from 1 to n,
  // by increments of 1 (where n is the total number of pages
  // required to store the payload for that fee)
  for (auto ob : pageCounts) {
    uint32_t orbit = static_cast<uint32_t>(ob.first & 0xFFFF);
    uint16_t bc = static_cast<uint16_t>((ob.first >> 32) & 0xFFFF);
    for (auto p : ob.second) {
      if (p.second.size() > 1) {
        for (auto i = 0; i < p.second.size() - 1; i++) {
          if (p.second[i] != p.second[i + 1] - 1) {
            ok = false;
          }
        }
      }
    }
  }
  return ok;
}

/// Check that the RDHs corresponding to IR=ir have
/// the TF bit of their triggerType set
template <typename RDH>
bool checkTimeFrameBitIsSet(gsl::span<const uint8_t> pages,
                            const o2::InteractionRecord& ir)
{
  int nset{0};
  int nir{0};
  forEachRDH<RDH>(pages, [&nset, &nir, &ir](const RDH& rdh) {
    if (rdhOrbit(rdh) == ir.orbit &&
        rdhBunchCrossing(rdh) == ir.bc) {
      nir++;
    }
    auto triggerType = rdhTriggerType(rdh);
    if (triggerType & o2::trigger::TF) {
      nset++;
    }
  });
  bool irFound = (nir > 0);
  bool allTFset = (nset == nir);
  return irFound && allTFset;
}

template <typename RDH>
std::vector<o2::InteractionRecord> getInteractions(gsl::span<uint8_t> pages)
{
  std::set<o2::InteractionRecord> irs;
  forEachRDH<RDH>(pages, [&irs](const RDH& rdh) {
    irs.emplace(rdhBunchCrossing(rdh), rdhOrbit(rdh));
  });
  std::vector<o2::InteractionRecord> interactions(irs.begin(), irs.end());
  return interactions;
}

template <typename RDH>
bool checkOrbitJumpsAreFullOrbits(gsl::span<uint8_t> pages)
{
  bool ok{true};
  auto irs = getInteractions<RDH>(pages);
  auto current = irs[0];
  for (auto i = 1; i < irs.size(); i++) {
    bool localok{true};
    if (irs[i].orbit != current.orbit) {
      if (irs[i].bc != current.bc) {
        localok = false;
        ok = false;
      } else {
        current = irs[i];
      }
    }
  }
  return ok;
}

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(encoder)

BOOST_AUTO_TEST_CASE_TEMPLATE(CheckOrbitJumpsAreAnIntegerNumberOfFullOrbits, RDH, testTypes)
{
  for (auto s : F.testPoints) {
    std::vector<uint8_t> pages;
    o2::mch::raw::normalizeBuffer<RDH>(F.buffer, pages, F.interactions, s.pageSize, F.paddingByte);
    bool ok = checkOrbitJumpsAreFullOrbits<RDH>(pages);
    BOOST_CHECK_EQUAL(ok, true);
  }
}

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

BOOST_AUTO_TEST_CASE_TEMPLATE(CheckPacketCounter, RDH, testTypes)
{
  for (auto s : F.testPoints) {
    std::vector<uint8_t> pages;
    o2::mch::raw::normalizeBuffer<RDH>(F.buffer, pages, F.interactions, s.pageSize, F.paddingByte);
    bool ok = packetCounterMustBeIncreasingByOne<RDH>(pages);
    BOOST_CHECK_EQUAL(ok, true);
  }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(FirstInteractionRecordMustHaveTFBitSet, RDH, testTypes)
{
  for (auto s : F.testPoints) {
    std::vector<uint8_t> pages;
    o2::mch::raw::normalizeBuffer<RDH>(F.buffer, pages, F.interactions, s.pageSize, F.paddingByte);
    bool ok = checkTimeFrameBitIsSet<RDH>(pages, F.interactions[0]);
    BOOST_CHECK_EQUAL(ok, true);
  }
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
