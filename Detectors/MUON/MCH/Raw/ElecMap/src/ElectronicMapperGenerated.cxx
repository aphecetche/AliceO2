// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHRawElecMap/ElectronicMapperGenerated.h"
#include "ElectronicMapperImplHelper.h"
#include <map>
#include <fmt/format.h>
#include "MCHRawElecMap/DsElecId.h"
#include "MCHRawElecMap/DsDetId.h"

// #include "GenCh5R.cxx"
// #include "GenCh5L.cxx"
// #include "GenCh6R.cxx"
// #include "GenCh6L.cxx"

// namespace
// {
// struct ElectronicMapperGeneratedImpl {
//   ElectronicMapperGeneratedImpl();
//   std::map<uint16_t, uint32_t> mDsElecId2DsDetId;
//   std::map<uint16_t, std::set<uint16_t>> mCruId2SolarId;
// };
//
// ElectronicMapperGeneratedImpl::ElectronicMapperGeneratedImpl()
//   : mDsElecId2DsDetId{},
//     mCruId2SolarId{}
// {
//   auto solars = fillElec2DetMap(mDsElecId2DsDetId);
//   fillCru2SolarMap(mCruId2SolarId, solars);
// }
//
// } // namespace
namespace o2::mch::raw
{
//
// static const ElectronicMapperGeneratedImpl& mapper()
// {
//   static ElectronicMapperGeneratedImpl mapper;
//   return mapper;
// }
//
template <>
std::function<std::optional<DsDetId>(DsElecId)>
  createElec2DetMapper<ElectronicMapperGenerated>(gsl::span<int> deIds,
                                                  uint64_t timestamp)
{
  std::map<uint16_t, uint32_t> e2d;
  return impl::mapperElec2Det<ElectronicMapperGenerated>(e2d);
}

// template <>
// std::function<std::optional<DsElecId>(DsDetId)>
//   mapperDet2Elec<ElectronicMapperGenerated>()
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
//   mapperCru2Solar<ElectronicMapperGenerated>()
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
//   mapperSolar2Cru<ElectronicMapperGenerated>()
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
