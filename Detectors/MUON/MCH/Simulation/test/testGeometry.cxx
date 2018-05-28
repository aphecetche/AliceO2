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
#include "../src/Geometry.h"
#include "TGeoManager.h"
#include <iomanip>

namespace bdata = boost::unit_test::data;

struct GEOMETRY {
  GEOMETRY()
  {
    if (!gGeoManager) {
      TGeoManager* g = new TGeoManager("TEST", "MCH");
      g->SetTopVolume(g->MakeBox("cave", nullptr, 2000.0, 2000.0, 3000.0));
      o2::mch::createGeometry();
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
  std::vector<std::string> chamberNames{quadrantChamberNames.begin(),quadrantChamberNames.end()};

  chamberNames.insert(chamberNames.end(),slatChamberNames.begin(),slatChamberNames.end());

  for (auto chname : chamberNames) {
    auto vol = gGeoManager->GetVolume(chname.c_str());
    BOOST_TEST_REQUIRE((vol != nullptr));
  }
}

BOOST_AUTO_TEST_CASE(GetRightNumberOfSlats)
{
  int nslats{ 0 };

  for (auto chname : slatChamberNames) {
    auto vol = gGeoManager->GetVolume(chname.c_str());
    TIter next(vol->GetNodes());
    while (TGeoNode* node = static_cast<TGeoNode*>(next())) {
      if (strstr(node->GetName(), "support") == nullptr) {
        nslats++;
      }
    }
  }
  BOOST_CHECK_EQUAL(nslats, 140);
}

BOOST_AUTO_TEST_CASE(GetRightNumberOfQuadrants)
{
  int nquads{ 0 };

  for (auto chname : quadrantChamberNames) {
    auto vol = gGeoManager->GetVolume(chname.c_str());
    TIter next(vol->GetNodes());
    while (TGeoNode* node = static_cast<TGeoNode*>(next())) {
      if (strstr(node->GetName(), "support") == nullptr) {
        nquads++;
      }
    }
  }
  BOOST_CHECK_EQUAL(nquads, 16);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()