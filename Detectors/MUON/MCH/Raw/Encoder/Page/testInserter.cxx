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

#define BOOST_TEST_MODULE Test MCHRaw Inserter
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include "DumpBuffer.h"
#include "FeeIdRange.h"
#include "Inserter.h"
#include <array>
#include <boost/mpl/list.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <fmt/format.h>
#include <fstream>
#include <gsl/span>
#include <iostream>
#include "Paginator.h"
#include "MCHRawEncoder/DataBlock.h"

using namespace o2::mch::raw;
using namespace o2::header;

namespace
{

std::vector<o2::InteractionRecord> interactions{
  o2::InteractionRecord{1, 100},
  o2::InteractionRecord{2, 210},
  o2::InteractionRecord{3, 360}};

std::vector<uint16_t> feeIds{10, 12, 11, 5, 3};

std::vector<std::byte> createBuffer(gsl::span<o2::InteractionRecord> interactions)
{
  std::vector<std::byte> buffer;
  for (auto ir : interactions) {
    for (auto feeId : feeIds) {
      DataBlockHeader header{ir.orbit, ir.bc, feeId, 0};
      appendDataBlockHeader(buffer, header);
    }
  }
  // add a few more to get an initial buffer with different
  // number of links per orbit
  appendDataBlockHeader(buffer, DataBlockHeader{interactions[0].orbit, static_cast<uint16_t>(interactions[0].bc + static_cast<uint16_t>(10)), feeIds[0], 0});
  if (interactions.size() > 1) {
    appendDataBlockHeader(buffer, DataBlockHeader{interactions[1].orbit, static_cast<uint16_t>(interactions[1].bc + static_cast<uint16_t>(20)), feeIds[1], 0});
  }

  return buffer;
} // namespace

std::vector<std::byte> testBuffer(gsl::span<o2::InteractionRecord> interactions)
{
  auto buffer = createBuffer(interactions);

  std::vector<std::byte> outBuffer;
  std::set<uint16_t> links(feeIds.begin(), feeIds.end());
  o2::InteractionRecord currentIR{42, 12345};
  equalizeHBFPerFeeId(buffer, outBuffer, links, currentIR);
  outBuffer.swap(buffer);

  return buffer;
}

} // namespace

bool checkAllFeesHaveSameNumberOfHeaders(gsl::span<const std::byte> buffer)
{
  std::map<uint16_t, std::vector<DataBlockRef>> headers;

  forEachDataBlockRef(buffer, [&headers](const DataBlockRef& ref) {
    headers[ref.block.header.solarId].emplace_back(ref);
  });

  bool first{true};
  int nref{0};

  for (auto p : headers) {
    if (first) {
      nref = p.second.size();
      first = false;
    } else {
      if (p.second.size() != nref) {
        return false;
      }
    }
  }
  return true;
}

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(encoder)

BOOST_AUTO_TEST_CASE(AllfeesMustHaveTheSameNumberOfHeaders)
{
  auto buffer = testBuffer(interactions);
  bool ok = checkAllFeesHaveSameNumberOfHeaders(buffer);
  BOOST_CHECK_EQUAL(ok, true);
}

BOOST_AUTO_TEST_CASE(NofRDHsMustBeNfeesTimesNHBs)
{
  auto buffer = testBuffer(interactions);

  int ntotal = countHeaders(buffer);

  auto nofHBs = 3 + interactions.size();
  auto expectedNofHeaders = nofHBs * feeIds.size();

  BOOST_CHECK_EQUAL(ntotal, expectedNofHeaders);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
