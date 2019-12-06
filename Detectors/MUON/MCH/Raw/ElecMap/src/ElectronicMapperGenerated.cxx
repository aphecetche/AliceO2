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
#include "MCHRawElecMap/DsElecId.h"
#include "MCHRawElecMap/DsDetId.h"
#include <iostream>
#include <fmt/format.h>

extern void fillElec2DetCH5R(std::map<uint16_t, uint32_t>& e2d);
extern void fillElec2DetCH5L(std::map<uint16_t, uint32_t>& e2d);
extern void fillElec2DetCH6R(std::map<uint16_t, uint32_t>& e2d);
extern void fillElec2DetCH6L(std::map<uint16_t, uint32_t>& e2d);

namespace o2::mch::raw
{

void dump(const std::map<uint16_t, uint32_t>& e2d)
{
  for (auto p : e2d) {
    std::cout << decodeDsElecId(p.first) << " -> " << decodeDsDetId(p.second) << "\n";
  }
}

// return a version of m where only the detection elements in deIds
// are present
std::map<uint16_t, uint32_t> filter(const std::map<uint16_t, uint32_t>& m, gsl::span<int> deIds)
{
  std::map<uint16_t, uint32_t> e2d;
  for (auto p : m) {
    DsDetId id = decodeDsDetId(p.second);
    if (std::find(deIds.begin(), deIds.end(), id.deId()) != deIds.end()) {
      e2d.emplace(p.first, p.second);
    }
  }
  return e2d;
}

std::map<uint16_t, uint32_t> buildDsElecId2DsDetIdMap(gsl::span<int> deIds)
{
  std::map<uint16_t, uint32_t> e2d;
  fillElec2DetCH5R(e2d);
  fillElec2DetCH5L(e2d);
  fillElec2DetCH6R(e2d);
  fillElec2DetCH6L(e2d);
  return filter(e2d, deIds);
}

template <>
std::function<std::optional<DsDetId>(DsElecId)>
  createElec2DetMapper<ElectronicMapperGenerated>(gsl::span<int> deIds, uint64_t timestamp)
{
  std::map<uint16_t, uint32_t> dsElecId2DsDetId = buildDsElecId2DsDetIdMap(deIds);
  return impl::mapperElec2Det<ElectronicMapperGenerated>(dsElecId2DsDetId);
}

template <>
std::function<std::optional<DsElecId>(DsDetId)>
  createDet2ElecMapper<ElectronicMapperGenerated>(gsl::span<int> deIds)
{
  std::map<uint16_t, uint32_t> dsElecId2DsDetId = buildDsElecId2DsDetIdMap(deIds);
  std::map<uint32_t, uint16_t> dsDetId2dsElecId;

  for (auto p : dsElecId2DsDetId) {
    dsDetId2dsElecId.emplace(p.second, p.first);
  }
  return impl::mapperDet2Elec<ElectronicMapperGenerated>(dsDetId2dsElecId);
}

template <>
std::function<std::set<uint16_t>(uint16_t)>
  createCru2SolarMapper<ElectronicMapperGenerated>(gsl::span<int> deIds)
{
  std::cout << "IMPLEMENT ME!\n";
  return nullptr;
}

template <>
std::function<std::optional<uint16_t>(uint16_t)>
  createSolar2CruMapper<ElectronicMapperGenerated>(gsl::span<int> deIds)
{
  std::cout << "IMPLEMENT ME!\n";
  return nullptr;
}

} // namespace o2::mch::raw
