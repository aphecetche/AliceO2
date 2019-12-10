// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHRawElecMap/ElectronicMapperDummy.h"
#include <map>
#include "MCHMappingFactory/CreateSegmentation.h"
#include <fmt/format.h>
#include "MCHRawElecMap/DsElecId.h"
#include "MCHRawElecMap/DsDetId.h"
#include "ElectronicMapperImplHelper.h"

namespace
{
// build the map to go from electronic ds id to detector ds id

std::map<uint16_t, uint32_t> buildDsElecId2DsDetIdMap(gsl::span<int> deIds)
{
  std::map<uint16_t, uint32_t> e2d;

  uint16_t n{0};
  uint16_t solarId{0};
  uint8_t groupId{0};
  uint8_t index{0};

  o2::mch::mapping::forEachDetectionElement([&](int deId) {
    auto seg = o2::mch::mapping::segmentation(deId);
    // assign a tuple (solarId,groupId,index) to the pair (deId,dsId)
    seg.forEachDualSampa([&](int dsId) {
      // index 0..4
      // groupId 0..7
      // solarId 0..nsolars
      if (n % 5 == 0) {
        index = 0;
        if (n % 8 == 0) {
          groupId = 0;
        } else {
          groupId++;
        }
      } else {
        index++;
      }
      if (n % 40 == 0) {
        solarId++;
      }
      o2::mch::raw::DsElecId dsElecId(solarId, groupId, index);
      o2::mch::raw::DsDetId dsDetId(deId, dsId);
      // only update the map if deId is one of the desired ones
      if (std::find(deIds.begin(), deIds.end(), deId) != deIds.end()) {
        e2d.emplace(o2::mch::raw::encode(dsElecId),
                    o2::mch::raw::encode(dsDetId));
      }
      n++;
    });
  });
  return e2d;
}

std::map<uint16_t, std::set<uint16_t>> buildSolarId2CruIdMap()
{
  std::map<uint16_t, std::set<uint16_t>> c2s;

  uint16_t n{0};
  uint16_t solarId{0};

  o2::mch::mapping::forEachDetectionElement([&](int deId) {
    auto seg = o2::mch::mapping::segmentation(deId);
    // assign a tuple (solarId,groupId,index) to the pair (deId,dsId)
    seg.forEachDualSampa([&](int dsId) {
      if (n % 40 == 0) {
        solarId++;
      }
      auto cruId = solarId / 24;
      c2s[cruId].insert(solarId);
      n++;
    });
  });
  return c2s;
}
} // namespace

namespace o2::mch::raw
{

template <>
std::function<std::optional<DsDetId>(DsElecId)>
  createElec2DetMapper<ElectronicMapperDummy>(gsl::span<int> deIds, uint64_t timestamp)
{
  std::map<uint16_t, uint32_t> dsElecId2DsDetId = buildDsElecId2DsDetIdMap(deIds);
  return impl::mapperElec2Det<ElectronicMapperDummy>(dsElecId2DsDetId);
}

template <>
std::function<std::optional<DsElecId>(DsDetId)>
  createDet2ElecMapper<ElectronicMapperDummy>(gsl::span<int> deIds)
{
  std::map<uint16_t, uint32_t> dsElecId2DsDetId = buildDsElecId2DsDetIdMap(deIds);
  std::map<uint32_t, uint16_t> dsDetId2dsElecId;

  for (auto p : dsElecId2DsDetId) {
    dsDetId2dsElecId.emplace(p.second, p.first);
  }

  return impl::mapperDet2Elec<ElectronicMapperDummy>(dsDetId2dsElecId);
}

template <>
std::function<std::set<uint16_t>(uint16_t)>
  createCru2SolarMapper<ElectronicMapperDummy>()
{
  auto c2s = buildSolarId2CruIdMap();
  return impl::mapperCru2Solar<ElectronicMapperDummy>(c2s);
}

template <>
std::function<std::optional<uint16_t>(uint16_t)>
  createSolar2CruMapper<ElectronicMapperDummy>()
{
  std::map<uint16_t, uint16_t> s2c;
  auto c2s = buildSolarId2CruIdMap();
  for (auto p : c2s) {
    for (auto s : p.second) {
      s2c[s] = p.first;
    }
  }
  return impl::mapperSolar2Cru<ElectronicMapperDummy>(s2c);
}
} // namespace o2::mch::raw
