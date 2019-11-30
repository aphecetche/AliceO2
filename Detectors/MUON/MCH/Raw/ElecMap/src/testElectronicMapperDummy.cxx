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
#include <fmt/format.h>
#include <set>
#include <boost/mpl/list.hpp>

using namespace o2::mch::raw;

typedef boost::mpl::list<o2::mch::raw::ElectronicMapperDummy /*,
                         o2::mch::raw::ElectronicMapperGenerated*/
                         >
  testTypes;

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(electronicmapperdummy)

BOOST_AUTO_TEST_CASE_TEMPLATE(MustContainAll16828DualSampas, T, testTypes)
{
  std::set<int> ds;

  auto d2e = o2::mch::raw::mapperDet2Elec<T>();

  o2::mch::mapping::forEachDetectionElement([&ds, d2e](int deid) {
    auto seg = o2::mch::mapping::segmentation(deid);
    seg.forEachDualSampa([&seg, &ds, d2e, deid](int dsid) {
      auto dsel = d2e(DsDetId{static_cast<uint16_t>(deid), static_cast<uint16_t>(dsid)});
      if (dsel.has_value()) {
        ds.insert(o2::mch::raw::encode(dsel.value())); // encode to be sure we're counting unique pairs (deid,dsid)
      }
    });
  });
  BOOST_CHECK_EQUAL(ds.size(), 16828);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(MustGetACruIdForEachDeId, T, testTypes)
{
  std::set<int> missing;

  auto de2cru = o2::mch::raw::mapperDe2Cru<T>();

  o2::mch::mapping::forEachDetectionElement([&](int deid) {
    auto cru = de2cru(deid);
    if (!cru.has_value()) {
      missing.insert(deid);
    }
  });
  BOOST_CHECK_EQUAL(missing.size(), 0);
}

// BOOST_AUTO_TEST_CASE_TEMPLATE(EachCruHasLessThan960DualSampas, T, testTypes)
// {
//   std::map<int, int> nofDualSampaPerCRU;
//
//   o2::mch::mapping::forEachDetectionElement([&](int deid) {
//     auto cru = elecmap<T>().cruId(deid);
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
