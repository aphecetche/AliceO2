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
#include "../Payload/CruBufferCreator.h"
#include "Paginator.h"
#include "CommonConstants/Triggers.h"
#include "DetectorsRaw/HBFUtils.h"
#include "DetectorsRaw/RawFileWriter.h"
#include "MCHRawElecMap/Mapper.h"
#include <fstream>

using namespace o2::mch::raw;

extern std::ostream& operator<<(std::ostream&, const o2::header::RAWDataHeaderV4&);

typedef boost::mpl::list<o2::header::RAWDataHeaderV4> testTypes;

struct Common {
  constexpr static size_t pageSize = 128;
  constexpr static std::byte paddingByte = std::byte{0x44};
  std::vector<std::byte> buffer;
  std::vector<o2::InteractionTimeRecord> interactions;
  o2::InteractionTimeRecord firstIR;
  std::set<uint16_t> feeIds = {728, 361, 448};

  struct TestPoint {
    int pageSize, expectedNofRDHs;
  };
  std::vector<TestPoint> testPoints;
  Common() : buffer(test::CruBufferCreator<BareFormat, ChargeSumMode>::makeBuffer(3)),
             interactions{},
             testPoints{
               {8192, 18},
               {1024, 30},
               {512, 50},
               {128, 262}}
  {

    std::set<o2::InteractionTimeRecord> irs;
    // get the IRs directly from the created buffer
    forEachDataBlockRef(buffer, [&irs](const DataBlockRef& ref) {
      irs.emplace(o2::InteractionRecord{ref.block.header.bc, ref.block.header.orbit}, 0);
    });
    interactions.insert(interactions.end(), irs.begin(), irs.end());
    firstIR = *(interactions.begin());
  }
} F;

template <typename RDH>
bool packetCounterMustBeIncreasingByOne(gsl::span<const std::byte> pages)
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
bool pageCountsMustBeIncreasingByOne(gsl::span<const std::byte> pages)
{
  // pageCounts is a map (orbc -> (linkuniqueid,{pageCounts}))
  // where orb is a combination of orbit and bc
  // {pageCounts} is a vector of page counters
  std::map<uint64_t, std::map<int, std::vector<int>>> pageCounts;
  forEachRDH<RDH>(pages, [&pageCounts](const RDH& rdh) {
    uint64_t orbc =
      static_cast<uint64_t>(rdhOrbit(rdh)) |
      ((static_cast<uint64_t>(rdhBunchCrossing(rdh)) << 32));
    uint32_t uniqueLinkId = rdhFeeId(rdh) << 16 | rdhLinkId(rdh);
    (pageCounts[orbc])[uniqueLinkId].emplace_back(rdhPageCounter(rdh));
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
bool checkTimeFrameBitIsSet(gsl::span<const std::byte> pages,
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
std::vector<o2::InteractionRecord> getInteractions(gsl::span<std::byte> pages)
{
  std::set<o2::InteractionRecord> irs;
  forEachRDH<RDH>(pages, [&irs](const RDH& rdh) {
    irs.emplace(rdhBunchCrossing(rdh), rdhOrbit(rdh));
  });
  std::vector<o2::InteractionRecord> interactions(irs.begin(), irs.end());
  return interactions;
}

template <typename RDH>
bool checkOrbitJumpsAreFullOrbits(gsl::span<std::byte> pages)
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
    std::vector<std::byte> pages;
    o2::mch::raw::BufferNormalizer<RDH> normalizer(F.firstIR, F.feeIds, s.pageSize, F.paddingByte);
    normalizer.normalize(F.buffer, pages, F.firstIR);
    bool ok = checkOrbitJumpsAreFullOrbits<RDH>(pages);
    BOOST_CHECK_EQUAL(ok, true);
    break;
  }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(CheckNumberOfRDHs, RDH, testTypes)
{
  for (auto s : F.testPoints) {
    std::vector<std::byte> pages;
    o2::mch::raw::BufferNormalizer<RDH> normalizer(F.firstIR, F.feeIds, s.pageSize, F.paddingByte);
    normalizer.normalize(F.buffer, pages, F.firstIR);
    auto nrdhs = o2::mch::raw::countRDHs<RDH>(pages);
    BOOST_CHECK_EQUAL(nrdhs, s.expectedNofRDHs);
  }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(CheckPageCounter, RDH, testTypes)
{
  for (auto s : F.testPoints) {
    std::vector<std::byte> pages;
    o2::mch::raw::BufferNormalizer<RDH> normalizer(F.firstIR, F.feeIds, s.pageSize, F.paddingByte);
    normalizer.normalize(F.buffer, pages, F.firstIR);
    bool ok = pageCountsMustBeIncreasingByOne<RDH>(pages);
    BOOST_CHECK_EQUAL(ok, true);
  }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(CheckPacketCounter, RDH, testTypes)
{
  for (auto s : F.testPoints) {
    std::vector<std::byte> pages;
    o2::mch::raw::BufferNormalizer<RDH> normalizer(F.firstIR, F.feeIds, s.pageSize, F.paddingByte);
    normalizer.normalize(F.buffer, pages, F.firstIR);
    bool ok = packetCounterMustBeIncreasingByOne<RDH>(pages);
    BOOST_CHECK_EQUAL(ok, true);
  }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(FirstInteractionRecordMustHaveTFBitSet, RDH, testTypes)
{
  for (auto s : F.testPoints) {
    std::vector<std::byte> pages;
    o2::mch::raw::BufferNormalizer<RDH> normalizer(F.firstIR, F.feeIds, s.pageSize, F.paddingByte);
    normalizer.normalize(F.buffer, pages, F.firstIR);
    bool ok = checkTimeFrameBitIsSet<RDH>(pages, F.firstIR);
    BOOST_CHECK_EQUAL(ok, true);
  }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TestBasicRawWriter, RDH, testTypes)
{
  for (auto s : F.testPoints) {
    std::ofstream out(fmt::format("mch.{}.raw", s.pageSize));
    std::vector<std::byte> pages;
    o2::mch::raw::BufferNormalizer<RDH> normalizer(F.firstIR, F.feeIds, s.pageSize, F.paddingByte);
    normalizer.normalize(F.buffer, pages, F.firstIR);
    out.write(reinterpret_cast<char*>(&pages[0]), pages.size());
  }
}

//BOOST_TEST_DECORATOR(*boost::unit_test::disabled())
BOOST_AUTO_TEST_CASE_TEMPLATE(TestRawFileWriter, RDH, testTypes)
{
  o2::raw::RawFileWriter fw;
  fw.setDontFillEmptyHBF(true);

  std::set<DataBlockRef> dataBlockRefs;

  forEachDataBlockRef(
    F.buffer, [&dataBlockRefs](const DataBlockRef& ref) {
      dataBlockRefs.insert(ref);
    });

  std::set<FeeLinkId> feeLinkIds;
  auto solar2feelink = o2::mch::raw::createSolar2FeeLinkMapper<ElectronicMapperGenerated>();

  for (auto r : dataBlockRefs) {
    feeLinkIds.insert(solar2feelink(r.block.header.solarId).value());
  }

  for (auto f : feeLinkIds) {
    int endpoint = f.feeId() % 2;
    int cru = (f.feeId() - endpoint) / 2;
    auto& link = fw.registerLink(f.feeId(), cru, f.linkId(), endpoint, "mch.raw");
    o2::conf::ConfigurableParam::setValue<uint32_t>("HBFUtils", "orbitFirst", F.firstIR.orbit);
    o2::conf::ConfigurableParam::setValue<uint16_t>("HBFUtils", "bcFirst", F.firstIR.bc);
  }

  for (auto r : dataBlockRefs) {
    auto& b = r.block;
    auto& h = b.header;
    auto f = solar2feelink(r.block.header.solarId).value();
    int endpoint = f.feeId() % 2;
    int cru = (f.feeId() - endpoint) / 2;
    std::cout << fmt::format("feeId {} cruId {} linkId {} endpoint {}\n",
                             f.feeId(), cru, f.linkId(), endpoint);
    fw.addData(f.feeId(), cru, f.linkId(), endpoint, {h.bc, h.orbit}, gsl::span<char>(const_cast<char*>(reinterpret_cast<const char*>(&b.payload[0])), b.payload.size()));
  }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(NormalizerOnEmptyBufferGetAtLeastOnePage, RDH, testTypes)
{
  std::vector<std::byte> buffer;
  std::vector<std::byte> pages;
  o2::mch::raw::BufferNormalizer<RDH> normalizer(F.firstIR, F.feeIds);
  normalizer.normalize(F.buffer, pages, {});
  BOOST_REQUIRE_GE(pages.size(), 1);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
