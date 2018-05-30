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

#define BOOST_TEST_MODULE Test MCHSimulation Geometry
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

#include "boost/format.hpp"
#include <boost/test/data/test_case.hpp>
#include <fstream>
#include <iostream>
#include "TGeoManager.h"
#include <iomanip>
#include "MCHMappingInterface/Segmentation.h"
#include "MCHSimulation/GeometryTransformer.h"
#include "MCHSimulation/Radiographer.h"
#include "MCHSimulation/Geometry.h"

namespace bdata = boost::unit_test::data;

struct GEOMETRY {
  GEOMETRY()
  {
    if (!gGeoManager) {
      TGeoManager* g = new TGeoManager("TEST", "MCH");
      TGeoVolume* top = o2::mch::createAirVacuumCave("cave");
      g->SetTopVolume(top);
      o2::mch::createGeometry(*top);
    }
  }
};

const std::array<std::string, 8> quadrantChamberNames{ "SC01I", "SC01O", "SC02I", "SC02O", "SC03I", "SC03O",
                                                       "SC04I", "SC04O" };

const std::array<std::string, 12> slatChamberNames{ "SC05I", "SC05O", "SC06I", "SC06O", "SC07I", "SC07O",
                                                    "SC08I", "SC08O", "SC09I", "SC09O", "SC10I", "SC10O" };

BOOST_AUTO_TEST_SUITE(o2_mch_simulation)

BOOST_FIXTURE_TEST_SUITE(geometrytransformer, GEOMETRY)

BOOST_AUTO_TEST_CASE(CanGetAllChambers)
{
  std::vector<std::string> chamberNames{ quadrantChamberNames.begin(), quadrantChamberNames.end() };

  chamberNames.insert(chamberNames.end(), slatChamberNames.begin(), slatChamberNames.end());

  for (auto chname : chamberNames) {
    auto vol = gGeoManager->GetVolume(chname.c_str());
    BOOST_TEST_REQUIRE((vol != nullptr));
  }
}

std::vector<TGeoNode*> getSlatNodes()
{
  std::vector<TGeoNode*> slats;
  for (auto chname : slatChamberNames) {
    auto vol = gGeoManager->GetVolume(chname.c_str());
    TIter next(vol->GetNodes());
    while (TGeoNode* node = static_cast<TGeoNode*>(next())) {
      if (strstr(node->GetName(), "support") == nullptr) {
        slats.push_back(node);
      }
    }
  }
  return slats;
}

std::vector<TGeoNode*> getQuadrantNodes()
{
  std::vector<TGeoNode*> quadrants;
  for (auto chname : quadrantChamberNames) {
    auto vol = gGeoManager->GetVolume(chname.c_str());
    TIter next(vol->GetNodes());
    while (TGeoNode* node = static_cast<TGeoNode*>(next())) {
      quadrants.push_back(node);
    }
  }
  return quadrants;
}

BOOST_AUTO_TEST_CASE(GetRightNumberOfSlats)
{
  auto slats = getSlatNodes();
  BOOST_CHECK_EQUAL(slats.size(), 140);
}

BOOST_AUTO_TEST_CASE(GetRightNumberOfQuadrants)
{
  auto quadrants = getQuadrantNodes();
  BOOST_CHECK_EQUAL(quadrants.size(), 16);
}

BOOST_AUTO_TEST_CASE(GetDetElemVolumePath, *boost::unit_test::disabled() * boost::unit_test::label("debug"))
{
  TIter next(gGeoManager->GetTopNode()->GetNodes());
  TGeoNode* node;
  TGeoNode* n2;

  std::vector<std::string> codeLines;

  while ((node = static_cast<TGeoNode*>(next()))) {
    std::cout << node->GetName() << "\n";
    TIter next2(node->GetNodes());
    while ((n2 = static_cast<TGeoNode*>(next2()))) {
      std::string n2name{ n2->GetName() };
      auto index = n2name.find_last_of('_');
      int detElemId = std::atoi(n2name.substr(index + 1).c_str());
      if (detElemId >= 100) {
        std::stringstream s;
        s << "if (detElemId==" << detElemId << ") {\n";
        s << "  return \"" << node->GetName() << "/" << n2name << "\";\n";
        s << "}\n";
        codeLines.push_back(s.str());
      }
    }
  }

  for (auto s : codeLines) {
    std::cout << s;
  }
  BOOST_CHECK_EQUAL(codeLines.size(), 156);
}

BOOST_AUTO_TEST_CASE(GetTransformations)
{
  BOOST_REQUIRE(gGeoManager != nullptr);

  o2::mch::mapping::forEachDetectionElement([](int detElemId) {
    BOOST_CHECK_NO_THROW((o2::mch::getTransformation(detElemId, *gGeoManager)));
  });
}

BOOST_AUTO_TEST_CASE(RadLen)
{
  // HERE SHOULD CREATE A COARSE HISTOGRAM OF RADLEN FOR ONE DE
  // and COMPARE WITH A REFERENCE ?

  auto h = o2::mch::getRadio(1025,o2::mch::contour::BBox<float>{-120,-20,120,20},1,1);

  BOOST_CHECK(false);
}

BOOST_AUTO_TEST_CASE(TextualTreeDump)
{
   o2::mch::showGeometryAsTextTree("",3); 
   o2::mch::showGeometryAsTextTree("",2); 
   o2::mch::showGeometryAsTextTree("",1); 
   BOOST_CHECK(false);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()