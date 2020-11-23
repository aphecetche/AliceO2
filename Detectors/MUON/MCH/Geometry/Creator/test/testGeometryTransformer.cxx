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

#define BOOST_TEST_MODULE Test MCHSimulation GeometryTransformer
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>

#include "MCHGeometryCreator/Geometry.h"
#include "MCHGeometryCreator/GeometryTest.h"
#include "MCHGeometryTransformer/Transformations.h"
#include "TGeoManager.h"
#include "boost/format.hpp"
#include <boost/test/data/test_case.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <fmt/format.h>

namespace bdata = boost::unit_test::data;

BOOST_TEST_DONT_PRINT_LOG_VALUE(o2::mch::geo::TransformationCreator)

o2::mch::geo::TransformationCreator transformation(int i)
{
  static std::vector<o2::mch::geo::TransformationCreator> vtrans;

  BOOST_REQUIRE(boost::unit_test::framework::master_test_suite().argc == 2);

  if (vtrans.empty()) {
    if (!gGeoManager) {
      o2::mch::test::createStandaloneGeometry();
      o2::mch::geo::addAlignableVolumes();
    }
    BOOST_TEST_REQUIRE(boost::unit_test::framework::master_test_suite().argc == 2);
    std::string jsonInput = boost::unit_test::framework::master_test_suite().argv[1];
    std::ifstream in(jsonInput);
    vtrans = {
      o2::mch::geo::transformationFromJSON(in),
      o2::mch::geo::transformationFromTGeoManager(*gGeoManager)
    };
  }

  return vtrans[i];
}

BOOST_DATA_TEST_CASE(GetTransformationMustNotThrowForValidDetElemId, bdata::xrange(2), tindex)
{
  for (auto detElemId : o2::mch::geo::allDeIds) {
    BOOST_REQUIRE_NO_THROW((transformation(tindex))(detElemId));
  }
}

BOOST_DATA_TEST_CASE(GetTransformationMustThrowForInvalidDetElemId, bdata::xrange(2), tindex)
{
  const auto someInvalidDetElemIds = {99, 105, 1026};

  for (auto detElemId : someInvalidDetElemIds) {
    BOOST_CHECK_THROW((transformation(tindex)(detElemId)), std::runtime_error);
  }
}

struct CoarseLocation {
  bool isRight;
  bool isTop;
};

constexpr CoarseLocation topRight{true, true};
constexpr CoarseLocation topLeft{false, true};
constexpr CoarseLocation bottomRight{true, false};
constexpr CoarseLocation bottomLeft{false, false};

std::string asString(const CoarseLocation& q)
{
  std::string s = q.isTop ? "TOP" : "BOTTOM";
  s += q.isRight ? "RIGHT" : "LEFT";
  return s;
}

bool operator==(const CoarseLocation& a, const CoarseLocation& b)
{
  return a.isRight == b.isRight && a.isTop == b.isTop;
}

CoarseLocation getDetElemCoarseLocation(int detElemId, o2::mch::geo::TransformationCreator transformation)
{
  auto t = transformation(detElemId);
  o2::math_utils::Point3D<double> localTestPos{0.0, 0.0, 0.0}; // slat center

  if (detElemId < 500) {
    // in the rough ballpark of the center
    // of the quadrants
    localTestPos.SetXYZ(60, 60, 0);
  }

  // for slats around the middle (y closest to 0) we have to be a bit
  // more precise, so take a given pad reference, chosen to be
  // the most top right or most top left pad

  switch (detElemId) {
    case 500:
    case 509:
      localTestPos.SetXYZ(-72.50, 19.75, 0.0); // ds 107
      break;
    case 600:
    case 609:
      localTestPos.SetXYZ(-77.50, 19.75, 0.0); // ds 108
      break;
    case 700:
    case 713:
    case 800:
    case 813:
    case 900:
    case 913:
    case 1000:
    case 1013:
      localTestPos.SetXYZ(95.0, -19.75, 0); // ds 104
      break;
  }
  o2::math_utils::Point3D<double> master;

  t.LocalToMaster(localTestPos, master);
  bool right = master.x() > 10.;
  bool top = master.y() > -10.;

  return CoarseLocation{right, top};
}

void setExpectation(int firstDeId, int lastDeId, CoarseLocation q, std::map<int, CoarseLocation>& expected)
{
  for (int deid = firstDeId; deid <= lastDeId; deid++) {
    expected.emplace(deid, q);
  }
}

BOOST_DATA_TEST_CASE(DetectionElementMustBeInTheRightCoarseLocation, bdata::xrange(2), tindex)
{
  std::map<int, CoarseLocation> expected;

  for (int i = 0; i < 4; i++) {
    expected[100 + i * 100] = topRight;
    expected[101 + i * 100] = topLeft;
    expected[102 + i * 100] = bottomLeft;
    expected[103 + i * 100] = bottomRight;
  }

  // note that by convention we consider slats in the middle to be "top"
  for (int i = 0; i < 2; i++) {
    setExpectation(500 + i * 100, 504 + i * 100, topRight, expected);
    setExpectation(505 + i * 100, 509 + i * 100, topLeft, expected);
    setExpectation(510 + i * 100, 513 + i * 100, bottomLeft, expected);
    setExpectation(514 + i * 100, 517 + i * 100, bottomRight, expected);
  }

  for (int i = 0; i < 4; i++) {
    setExpectation(700 + i * 100, 706 + i * 100, topRight, expected);
    setExpectation(707 + i * 100, 713 + i * 100, topLeft, expected);
    setExpectation(714 + i * 100, 719 + i * 100, bottomLeft, expected);
    setExpectation(720 + i * 100, 725 + i * 100, bottomRight, expected);
  }

  for (auto detElemId : o2::mch::geo::allDeIds) {
    if (expected.find(detElemId) == expected.end()) {
      std::cout << "got no expectation for DE=" << detElemId << "\n";
      return;
    }
    BOOST_TEST_INFO(fmt::format("DeId {}", detElemId));
    BOOST_REQUIRE_EQUAL(asString(getDetElemCoarseLocation(detElemId, transformation(tindex))), asString(expected[detElemId]));
  };
}
