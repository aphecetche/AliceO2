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

#define BOOST_TEST_MODULE Test MCHRaw Counter
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include "DumpBuffer.h"
#include "Headers/RAWDataHeader.h"
#include "FeeIdRange.h"
#include "Counter.h"
#include "MCHRawCommon/RDHManip.h"
#include <array>
#include <boost/mpl/list.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <fmt/format.h>
#include <fstream>
#include <gsl/span>
#include <iostream>

using namespace o2::mch::raw;
using namespace o2::header;

typedef boost::mpl::list<o2::header::RAWDataHeaderV4> testTypes;

namespace
{
template <typename RDH>
std::vector<uint8_t> createBuffer()
{
  std::vector<uint8_t> buffer;
  std::vector<uint8_t> fees{1, 1, 1, 2, 2, 2};

  for (auto feeId : fees) {
    auto rdh = createRDH<RDH>(
      0, feeId, 0, 0, 0, 0);
    appendRDH(buffer, rdh);
  }
  return buffer;
}

template <typename RDH>
bool isPacketCounterContiguousPerfee(gsl::span<uint8_t> buffer)
{
  std::map<int, uint8_t> counters;
  bool ok{true};

  forEachRDH<RDH>(buffer, [&counters, &ok](const RDH& rdh) {
    auto feeId = rdhFeeId(rdh);
    if (counters.find(feeId) == counters.end()) {
      counters.emplace(feeId, 0);
      if (rdhPacketCounter(rdh) != 0) {
        ok = false;
      }
    } else {
      int c = counters[feeId];
      if (static_cast<int>(rdhPacketCounter(rdh)) != static_cast<int>(c + 1)) {
        ok = false;
      }
      counters[feeId]++;
    }
  });
  return ok;
}
} // namespace

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(arranger)

BOOST_AUTO_TEST_CASE_TEMPLATE(TestPacketCounter, T, testTypes)
{
  auto buffer = createBuffer<T>();

  setPacketCounter<T>(buffer);

  BOOST_CHECK_EQUAL(isPacketCounterContiguousPerfee<T>(buffer), true);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
