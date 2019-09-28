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
#include "MCHRaw/ELink.h"

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

void encode1(BitSet& bs, int& n)
{
  Encoder enc;

  n = 0;

  std::vector<int> chid = {1, 5, 13, 31};
  std::vector<int> chval = {10, 50, 130, 310};

  enc.appendOneDualSampa(bs, 9, 0, chid, chval);
  n += chid.size();

  chid = {1, 6, 14, 26, 27, 30};
  chval = {10, 60, 14, 260, 270, 300};

  n += chid.size();

  enc.appendOneDualSampa(bs, 3, 0, chid, chval);
}

int encode2(BitSet& bs, int& n)
{
  // generate a bitset, starting with m "garbage" bits
  // returns m

  // some garbage first
  int m = 13;
  for (int i = 0; i < m; i++) {
    bs.append(static_cast<bool>(rand() % 2));
  }
  // then a sync
  bs.append(sampaSync().uint64(), 50);
  // then the data
  encode1(bs, n);
  return m + 50;
}

BOOST_AUTO_TEST_CASE(EncodeOneDS)
{
  int n{0};
  BitSet bs;
  encode1(bs, n);
  BOOST_CHECK_EQUAL(bs.len(), n * (50 + 40));
}

BOOST_AUTO_TEST_CASE(TestELinkDecoding)
{
  int n{0};
  BitSet bs;

  // bs.append(sampaSync().uint64(), 50);
  // std::cout << sampaSync() << "\n";
  // std::cout << bs.stringLSBLeft() << "\n";
  // std::cout << bs.stringLSBRight() << "\n";
  //
  // bs.clear();
  int m = encode2(bs, n);
  BOOST_CHECK_EQUAL(bs.len(), m + n * (50 + 40));

  ELink e(12);

  for (int i = 0; i < bs.len() - 1; i += 2) {
    e.append(bs.get(i), bs.get(i + 1));
  }

  std::cout << "nof bits : " << bs.len() << "\n";
  std::cout << "elink=" << e << "\n";
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
