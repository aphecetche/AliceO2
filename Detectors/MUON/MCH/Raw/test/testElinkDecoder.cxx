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

#define BOOST_TEST_MODULE Test MCHRaw ElinkDecoder
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <fstream>
#include <fmt/printf.h>
#include "MCHRaw/ElinkDecoder.h"
#include "common.h"

using namespace o2::mch::raw;

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(elinkdecoder)

BOOST_AUTO_TEST_CASE(ELinkDecoderIdMustBeBetween0And39)
{
  BOOST_CHECK_THROW(ElinkDecoder dec(0, 40, test::handlePacketPrint("dummy")), std::invalid_argument);
  BOOST_CHECK_NO_THROW(ElinkDecoder dec(0, 39, test::handlePacketPrint("dummy")));
}

BOOST_AUTO_TEST_CASE(TestELinkDecoding)
{
  ElinkEncoder enc = o2::mch::raw::test::createElinkEncoder();
  int npackets{0};
  auto helper = test::handlePacketPrint("TestELinkDecoding:");

  auto hp = [&npackets, helper](SampaHit sh) {
    npackets++;
    helper(sh);
  };

  ElinkDecoder e(0, enc.id(), hp);

  for (int i = 0; i < enc.len() - 1; i += 2) {
    e.append(enc.get(i), enc.get(i + 1));
  }

  e.finalize();

  BOOST_CHECK_EQUAL(npackets, 4);

  // same thing but with a decoder without a channel handler
  // so we don't "see" any packet in this case
  npackets = 0;
  ElinkDecoder e2(0, enc.id(), nullptr);
  for (int i = 0; i < enc.len() - 1; i += 2) {
    e2.append(enc.get(i), enc.get(i + 1));
  }

  e2.finalize();

  BOOST_CHECK_EQUAL(npackets, 0);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
