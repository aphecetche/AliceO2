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
#include "Headers/RAWDataHeader.h"
#include "FeeIdRange.h"
#include "Inserter.h"
#include "MCHRawCommon/RDHManip.h"
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

typedef boost::mpl::list<o2::header::RAWDataHeaderV4> testTypes;

namespace
{

std::vector<o2::InteractionRecord> interactions{
  o2::InteractionRecord{1, 100},
  o2::InteractionRecord{2, 210},
  o2::InteractionRecord{3, 360}};

std::vector<o2::InteractionRecord> emptyHBs{
  {1, 12},
  {1, 99},
  {220, 220}};

std::vector<uint16_t> feeIds{10, 12, 11, 5, 3};

std::vector<uint8_t> createBuffer(gsl::span<o2::InteractionRecord> interactions)
{
  std::vector<uint8_t> buffer;
  for (auto ir : interactions) {
    for (auto feeId : feeIds) {
      PayloadHeader header{ir.orbit, ir.bc, feeId, 0};
      appendHeader(buffer, header);
    }
  }
  // add a few more to get an initial buffer with different
  // number of links per orbit
  appendHeader(buffer, PayloadHeader{interactions[0].orbit, interactions[0].bc, feeIds[0], 0});
  if (interactions.size() > 1) {
    appendHeader(buffer, PayloadHeader{interactions[1].orbit, interactions[1].bc, feeIds[1], 0});
  }

  return buffer;
} // namespace

template <typename RDH>
std::vector<uint8_t> testBuffer(gsl::span<o2::InteractionRecord> interactions)
{
  auto buffer = createBuffer(interactions);
  std::cout << "initial buffer size = " << buffer.size() << "\n";
  std::vector<uint8_t> pages;
  paginateBuffer<RDH>(buffer, pages, 80, 0xFF);

  impl::dumpBuffer(pages);

  std::vector<uint8_t> outBuffer;
  outBuffer.swap(pages);
  //insertEmptyHBs<RDH>(pages, outBuffer, emptyHBs);

  return outBuffer;
}

} // namespace

template <typename RDH>
bool checkAllFeesHaveSameNumberOfRDHs(gsl::span<const uint8_t> buffer)
{
  auto lr = getFeeIdRanges<RDH>(buffer);
  bool ok{true};
  bool first{true};
  size_t refSize;

  for (auto l : lr) {
    if (first) {
      refSize = l.second.size();
      first = false;
    }
    if (l.second.size() != refSize) {
      ok = false;
    }
  }
  return ok;
}

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(encoder)

BOOST_AUTO_TEST_CASE_TEMPLATE(AllfeesMustHaveTheSameNumberOfRDHs, RDH, testTypes)
{
  std::vector<o2::InteractionRecord> irs{
    o2::InteractionRecord{1, 100}};
  auto buffer = testBuffer<RDH>(irs);
  bool ok = checkAllFeesHaveSameNumberOfRDHs<RDH>(buffer);
  BOOST_CHECK_EQUAL(ok, true);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(NofRDHsMustBeNfeesTimesNHBs, RDH, testTypes)
{
  auto buffer = testBuffer<RDH>(interactions);

  int ntotal = countRDHs<RDH>(buffer);

  auto nofHBs = emptyHBs.size() + interactions.size();
  auto expectedNofRDHs = nofHBs * feeIds.size();

  BOOST_CHECK_EQUAL(ntotal, expectedNofRDHs);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
