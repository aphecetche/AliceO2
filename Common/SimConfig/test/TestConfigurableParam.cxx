// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#define BOOST_TEST_MODULE Test ConfigurableParam
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <CommonUtils/ConfigurableParam.h>
#include <CommonUtils/ConfigurableParamHelper.h>
#include <SimConfig/SimConfig.h>
#include <iostream>
#include <TInterpreter.h>
#include <array>
#include "BazParam.h"

using namespace o2;

struct INIT {
  INIT()
  {
    std::cout << "INIT\n";
    auto& conf = o2::conf::SimConfig::Instance();
    conf.resetFromArguments(boost::unit_test::framework::master_test_suite().argc,
                            boost::unit_test::framework::master_test_suite().argv);
    o2::conf::ConfigurableParam::updateFromString(conf.getKeyValueString());
  }
};

// struct GenerateDictionary {
//   GenerateDictionary()
//   {
//     std::cout << "Begin GenerateDictionary\n";
//     gInterpreter->GenerateDictionary("BazParam");
//     gInterpreter->GenerateTClass("BazParam", false);
//     std::cout << "End GenerateDictionary\n";
//   }
// };
//
//BOOST_TEST_GLOBAL_FIXTURE(GenerateDictionary);

BOOST_FIXTURE_TEST_SUITE(ConfigurableParam_test_suite, INIT)

BOOST_AUTO_TEST_CASE(PrintAllKeyValuePairs)
{
  // prints all parameters from any ConfigurableParam
  o2::conf::ConfigurableParam::printAllKeyValuePairs();
}

BOOST_AUTO_TEST_CASE(WriteIni)
{
  // writes a configuration file
  o2::conf::ConfigurableParam::writeINI("initialconf.ini");
}

BOOST_AUTO_TEST_CASE(QueryFromAPI)
{
  // query using C++ object + API
  auto d1 = BazParam::Instance().getGasDensity();
  BOOST_CHECK_EQUAL(d1, 42.42);
}

BOOST_AUTO_TEST_CASE(QueryFromRegistryAndStringName)
{
  // query from global parameter registry AND by string name
  auto d2 = o2::conf::ConfigurableParam::getValueAs<double>("Baz.mGasDensity");
  BOOST_CHECK_EQUAL(d2, 42.42);
}

BOOST_AUTO_TEST_CASE(Update)
{
  // update
  double x = 102;
  o2::conf::ConfigurableParam::setValue<double>("Baz", "mGasDensity", x);
  // check that update correctly synced
  auto d3 = o2::conf::ConfigurableParam::getValueAs<double>("Baz.mGasDensity");
  BOOST_CHECK_EQUAL(d3, x);
  BOOST_CHECK_EQUAL(d3, BazParam::Instance().getGasDensity());
  // BOOST_CHECK_EQUAL(x, BazParam::Instance().getGasDensity());
}

BOOST_AUTO_TEST_CASE(WriteUpdateIni)
{
  o2::conf::ConfigurableParam::writeINI("newconf.ini");
}

BOOST_AUTO_TEST_SUITE_END()
