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
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

#include "boost/format.hpp"
#include <boost/test/data/test_case.hpp>
#include <fstream>
#include <iostream>
#include "../src/SlatGeometry.h"

namespace bdata = boost::unit_test::data;

struct GEOMETRY {
   GEOMETRY() {
     o2::mch::createSlatGeometry();
     std::cout << "HERE\n";
   } 
};

BOOST_AUTO_TEST_SUITE(o2_mch_simulation) 
BOOST_FIXTURE_TEST_SUITE(geometrytransformer,GEOMETRY)

BOOST_AUTO_TEST_CASE(One)
{
  BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
