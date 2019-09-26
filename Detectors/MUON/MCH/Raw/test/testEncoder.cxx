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
#include "MCHRaw/SampaHeader.h"
#include "RAWDataHeader.h"
#include <fstream>
#include "MCHRaw/Encoder.h"
#include <fmt/printf.h>

using namespace o2::mch::raw;
using RDH = o2::Header::RAWDataHeader;

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(encoder)

BOOST_AUTO_TEST_CASE(GenerateRDH)
{
  std::array<std::byte, 8192> page;
  std::byte zero{0};
  std::fill(begin(page), end(page), zero);

  RDH rdh;
  int bc{12345};

  rdh.feeId = 42;
  rdh.linkId = 23;
  std::ofstream out("mch-test-rdh.dat");
  rdh.heartbeatBC = 1;
  rdh.offsetNextPacket = 8192;
  out.write((char*)&rdh, sizeof(rdh));
  out.write((char*)&page, sizeof(page) - sizeof(rdh));
  rdh.heartbeatBC = 2;
  rdh.triggerBC = ++bc;
  rdh.offsetNextPacket = sizeof(rdh);
  out.write((char*)&rdh, sizeof(rdh));
  rdh.triggerBC = ++bc;
  rdh.heartbeatBC = 3;
  out.write((char*)&rdh, sizeof(rdh));
}

BOOST_AUTO_TEST_CASE(EncodeOneDS)
{
  Encoder enc;

  std::array<int, 3> chid = {1, 5, 63};
  std::array<int, 3> chval = {10, 50, 630};
  auto bs = enc.oneDS(101, 0, chid, chval);
  BOOST_CHECK_EQUAL(bs.uint64(0, 49), sampaSync().asUint64());
}
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
