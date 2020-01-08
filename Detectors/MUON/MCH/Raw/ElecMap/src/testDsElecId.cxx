// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#define BOOST_TEST_MODULE Test MCHRaw DsElecId
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>

#include "MCHRawElecMap/DsElecId.h"
#include <fmt/format.h>

using namespace o2::mch::raw;

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(dselecid)

BOOST_AUTO_TEST_CASE(DsElecId)
{
  o2::mch::raw::DsElecId eid(448, 6, 2);
  BOOST_CHECK_EQUAL(asString(eid), "S448-J6-DS2");
  auto code = encode(eid);
  auto x = decodeDsElecId(code);
  BOOST_CHECK_EQUAL(code, encode(x));
}

BOOST_AUTO_TEST_CASE(DsElecIdValidString)
{
  o2::mch::raw::DsElecId eid(448, 6, 2);
  auto x = decodeDsElecId("S448-J6-DS2");
  BOOST_CHECK_EQUAL(encode(eid), encode(x));
}

BOOST_AUTO_TEST_CASE(DecoderDsElecIdShouldThrowForTooShortString)
{
  BOOST_CHECK_THROW(decodeDsElecId("S448"), std::invalid_argument);
  BOOST_CHECK_THROW(decodeDsElecId("S448-XX"), std::invalid_argument);
  BOOST_CHECK_NO_THROW(decodeDsElecId("S448-J3-DS2-CH-0"));
}

BOOST_AUTO_TEST_CASE(DecoderDsElecIdShouldThrowForInvalidSolar)
{
  BOOST_CHECK_THROW(decodeDsElecId("s448-J6-DS2"), std::invalid_argument);
  BOOST_CHECK_THROW(decodeDsElecId("448-J6-DS2"), std::invalid_argument);
  BOOST_CHECK_THROW(decodeDsElecId("X448-J6-DS2"), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(DecoderDsElecIdShouldThrowForInvalidGroup)
{
  BOOST_CHECK_THROW(decodeDsElecId("S448-6-DS2"), std::invalid_argument);
  BOOST_CHECK_THROW(decodeDsElecId("S448-j6-DS2"), std::invalid_argument);
  BOOST_CHECK_THROW(decodeDsElecId("S448-X6-DS2"), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(DecoderDsElecIdShouldThrowForInvalidDS)
{
  BOOST_CHECK_THROW(decodeDsElecId("S448-J6-DS"), std::invalid_argument);
  BOOST_CHECK_THROW(decodeDsElecId("S448-J6-Ds2"), std::invalid_argument);
  BOOST_CHECK_THROW(decodeDsElecId("S448-J6-D2"), std::invalid_argument);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
