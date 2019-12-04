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
struct ElectronicMapperDummyImpl {
  ElectronicMapperDummyImpl();
  std::map<uint16_t, uint32_t> mDsElecId2DsDetId;
  std::map<uint16_t, std::set<uint16_t>> mCruId2SolarId;
};

std::set<uint16_t> fillElec2DetMap(std::map<uint16_t, uint32_t>& e2d)
{
  /// e2d is a (fake) (solarId,groupId,index)->(deId,dsId) map
  ///
  /// to build this map we assume all solars have 8 groups (not true in reality)
  /// and that all groups have 5 dual sampas (i.e. index = 0..4) (which is
  /// also not true in reality).
  /// That way we end up with "only" 421 solars, while in reality we have
  /// more than 600.
  ///

  uint16_t n{0};
  uint16_t solarId{0};
  uint8_t groupId{0};
  uint8_t index{0};

  std::set<uint16_t> solars;

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
      e2d.emplace(o2::mch::raw::encode(dsElecId),
                  o2::mch::raw::encode(dsDetId));
      solars.insert(solarId);
      n++;
    });
  });

  return solars;
}

std::map<uint16_t, uint32_t> buildDsElecId2DsDetIdMap(gsl::span<int> deIds)
{
  std::map<uint16_t, uint32_t> e2d;

  uint16_t n{0};
  uint16_t solarId{0};
  uint8_t groupId{0};
  uint8_t index{0};

  for (auto deId : deIds) {
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
      e2d.emplace(o2::mch::raw::encode(dsElecId),
                  o2::mch::raw::encode(dsDetId));
      n++;
    });
  }
  return e2d;
}

void fillCru2SolarMap(std::map<uint16_t, std::set<uint16_t>>& c2s,
                      const std::set<uint16_t> solarIds)
{
  /// c2s is a (fake) map of cruId -> { solarId }
  /// we assume here that all CRUS are "full" = 6 solars / CRU
  uint16_t n{0};
  for (auto s : solarIds) {
    auto cruId = n / 6;
    c2s[cruId].insert(s);
    n++;
  }
}

ElectronicMapperDummyImpl::ElectronicMapperDummyImpl()
  : mDsElecId2DsDetId{},
    mCruId2SolarId{}
{
  auto solars = fillElec2DetMap(mDsElecId2DsDetId);
  fillCru2SolarMap(mCruId2SolarId, solars);
}

} // namespace

namespace o2::mch::raw
{
static const ElectronicMapperDummyImpl& mapper()
{
  static ElectronicMapperDummyImpl mapper;
  return mapper;
}

template <>
std::function<std::optional<DsDetId>(DsElecId)>
  createElec2DetMapper<ElectronicMapperDummy>(gsl::span<int> deids, uint64_t timestamp)
{
  std::map<uint16_t, uint32_t> dsElecId2DsDetId = buildDsElecId2DsDetIdMap(deids);
  return impl::mapperElec2Det<ElectronicMapperDummy>(dsElecId2DsDetId);
}
template <>
std::function<std::optional<DsElecId>(DsDetId)>
  createDet2ElecMapper<ElectronicMapperDummy>(gsl::span<int> deids)
{
  return nullptr;
}
template <>
std::function<std::set<uint16_t>(uint16_t)>
  createCru2SolarMapper<ElectronicMapperDummy>(gsl::span<int> deids)
{
  return nullptr;
}

// template <>
// std::function<std::optional<DsElecId>(DsDetId)>
//   mapperDet2Elec<ElectronicMapperDummy>()
// {
//   std::map<uint32_t, uint16_t> det2elec;
//
//   for (auto p : mapper().mDsElecId2DsDetId) {
//     det2elec.emplace(p.second, p.first);
//   }
//
//   return [det2elec](DsDetId id) -> std::optional<DsElecId> {
//     auto it = det2elec.find(encode(id));
//     if (it == det2elec.end()) {
//       return std::nullopt;
//     }
//     return decodeDsElecId(it->second);
//   };
// }
//
// template <>
// std::function<std::set<uint16_t>(uint16_t)>
//   mapperCru2Solar<ElectronicMapperDummy>()
// {
//   return [](uint16_t cruId) -> std::set<uint16_t> {
//     auto it = mapper().mCruId2SolarId.find(cruId);
//     if (it == mapper().mCruId2SolarId.end()) {
//       return {};
//     }
//     return it->second;
//   };
// }
//
// template <>
// std::function<std::optional<uint16_t>(uint16_t)>
//   mapperSolar2Cru<ElectronicMapperDummy>()
// {
//   std::map<uint16_t, uint16_t> solar2cru;
//
//   for (auto p : mapper().mCruId2SolarId) {
//     for (auto s : p.second) {
//       solar2cru.emplace(s, p.first);
//     }
//   }
//   return [solar2cru](uint16_t solarId) -> std::optional<uint16_t> {
//     auto it = solar2cru.find(solarId);
//     if (it == solar2cru.end()) {
//       return std::nullopt;
//     }
//     return it->second;
//   };
// }
} // namespace o2::mch::raw
