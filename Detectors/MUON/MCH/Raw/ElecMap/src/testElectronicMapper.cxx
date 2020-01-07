// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#define BOOST_TEST_MODULE Test MCHRaw CRUEncoder
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>

#include "MCHMappingFactory/CreateSegmentation.h"
#include "MCHRawElecMap/Mapper.h"
#include "MCHRawElecMap/ElectronicMapperDummy.h"
#include "MCHRawElecMap/ElectronicMapperGenerated.h"
#include <fmt/format.h>
#include <set>
#include <boost/mpl/list.hpp>
#include <gsl/span>
#include <array>

using namespace o2::mch::raw;

typedef boost::mpl::list<o2::mch::raw::ElectronicMapperDummy,
                         o2::mch::raw::ElectronicMapperGenerated>
  testTypes;

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(electronicmapperdummy)

template <size_t N>
std::set<int> nofDualSampas(std::array<int, N> deIds)
{
  std::set<int> ds;

  for (auto deId : deIds) {
    auto seg = o2::mch::mapping::segmentation(deId);
    seg.forEachDualSampa([&ds, deId](int dsid) {
      ds.insert(encode(DsDetId{deId, dsid}));
    });
  }
  return ds;
}

template <typename T>
std::set<int> nofDualSampasFromMapper(gsl::span<int> deids)
{
  std::set<int> ds;

  auto d2e = o2::mch::raw::createDet2ElecMapper<T>(deids);

  for (auto deid : deids) {
    auto seg = o2::mch::mapping::segmentation(deid);
    size_t nref = seg.nofDualSampas();
    std::set<int> dsForOneDE;
    seg.forEachDualSampa([&seg, &ds, d2e, deid, &dsForOneDE](int dsid) {
      DsDetId id{static_cast<uint16_t>(deid), static_cast<uint16_t>(dsid)};
      auto dsel = d2e(id);
      if (dsel.has_value()) {
        auto code = o2::mch::raw::encode(id); // encode to be sure we're counting unique pairs (deid,dsid)
        ds.insert(code);
        dsForOneDE.insert(code);
      }
    });
    if (dsForOneDE.size() != nref) {
      std::cout << fmt::format("ERROR : mapper has {:4d} DS while DE {:4d} should have {:4d}\n", dsForOneDE.size(), deid, nref);
    }
  }
  return ds;
}

BOOST_AUTO_TEST_CASE_TEMPLATE(MustContainAllSampaCH5R, T, testTypes)
{
  auto check = nofDualSampasFromMapper<T>(o2::mch::raw::deIdsOfCH5R);
  auto expected = nofDualSampas(o2::mch::raw::deIdsOfCH5R);
  BOOST_CHECK_EQUAL(check.size(), expected.size());
  BOOST_CHECK(std::equal(expected.begin(), expected.end(), check.begin()));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(MustContainAllSampaCH5L, T, testTypes)
{
  auto check = nofDualSampasFromMapper<T>(o2::mch::raw::deIdsOfCH5L);
  auto expected = nofDualSampas(o2::mch::raw::deIdsOfCH5L);
  BOOST_CHECK(std::equal(expected.begin(), expected.end(), check.begin()));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(MustContainAllSampaCH6R, T, testTypes)
{
  auto check = nofDualSampasFromMapper<T>(o2::mch::raw::deIdsOfCH6R);
  auto expected = nofDualSampas(o2::mch::raw::deIdsOfCH6R);
  BOOST_CHECK(std::equal(expected.begin(), expected.end(), check.begin()));
}

BOOST_TEST_DECORATOR(*boost::unit_test::disabled())
BOOST_AUTO_TEST_CASE_TEMPLATE(MustContainAllSampaCH6L, T, testTypes)
{
  auto check = nofDualSampasFromMapper<T>(o2::mch::raw::deIdsOfCH6L);
  auto expected = nofDualSampas(o2::mch::raw::deIdsOfCH6L);
  BOOST_CHECK(std::equal(expected.begin(), expected.end(), check.begin()));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(MustContainAllSampaCH7R, T, testTypes)
{
  auto check = nofDualSampasFromMapper<T>(o2::mch::raw::deIdsOfCH7R);
  auto expected = nofDualSampas(o2::mch::raw::deIdsOfCH7R);
  BOOST_CHECK(std::equal(expected.begin(), expected.end(), check.begin()));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(MustContainAllSampaCH7L, T, testTypes)
{
  auto check = nofDualSampasFromMapper<T>(o2::mch::raw::deIdsOfCH7L);
  auto expected = nofDualSampas(o2::mch::raw::deIdsOfCH7L);
  BOOST_CHECK(std::equal(expected.begin(), expected.end(), check.begin()));
}

BOOST_AUTO_TEST_CASE(DsElecId)
{
  o2::mch::raw::DsElecId eid(448, 6, 2);
  BOOST_CHECK_EQUAL(asString(eid), "S448-J6-DS2");
  auto code = encode(eid);
  auto x = decodeDsElecId(code);
  BOOST_CHECK_EQUAL(code, encode(x));
}

// BOOST_AUTO_TEST_CASE_TEMPLATE(MustGetACruIdForEachDeId, T, testTypes)
// {
//   std::set<int> missing;
//
//   auto de2cru = o2::mch::raw::mapperDe2Cru<T>();
//
//   o2::mch::mapping::forEachDetectionElement([&](int deid) {
//     auto cru = de2cru(deid);
//     if (!cru.has_value()) {
//       missing.insert(deid);
//     }
//   });
//   BOOST_CHECK_EQUAL(missing.size(), 0);
// }
//
// BOOST_AUTO_TEST_CASE_TEMPLATE(EachCruHasLessThan960DualSampas, T, testTypes)
// {
//   std::map<int, int> nofDualSampaPerCRU;
//
//   auto de2cru = o2::mch::raw::mapperDe2Cru<T>();
//
//   o2::mch::mapping::forEachDetectionElement([&](int deid) {
//     auto cru = de2cru(deid);
//     if (cru.has_value()) {
//       auto seg = o2::mch::mapping::segmentation(deid);
//       nofDualSampaPerCRU[cru.value()] += seg.nofDualSampas();
//     }
//   });
//   for (auto p : nofDualSampaPerCRU) {
//     BOOST_CHECK_LE(p.second, 40 * 24);
//   }
// }
//
// BOOST_AUTO_TEST_CASE_TEMPLATE(MustHaveAtLeast18Crus, T, testTypes)
// {
//   auto crus = elecmap<T>().cruIds();
//
//   BOOST_CHECK_GE(crus.size(), 19);
// }
//
// BOOST_AUTO_TEST_CASE_TEMPLATE(MustHaveConsistentNumberOfSolars, T, testTypes)
// {
//   auto crus = elecmap<T>().cruIds();
//   int nsolars{0};
//   for (auto cruId : crus) {
//     auto solars = elecmap<T>().solarIds(cruId);
//     nsolars += solars.size();
//   }
//   BOOST_CHECK_EQUAL(nsolars, elecmap<T>().nofSolars());
// }
//
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
