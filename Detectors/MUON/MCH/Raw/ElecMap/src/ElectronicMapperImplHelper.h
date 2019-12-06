// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_ELECMAP_ELECTRONIC_MAPPER_IMPL_HELPER_H
#define O2_MCH_RAW_ELECMAP_ELECTRONIC_MAPPER_IMPL_HELPER_H

#include "MCHRawElecMap/DsDetId.h"
#include "MCHRawElecMap/DsElecId.h"
#include <functional>
#include <optional>
#include <map>
#include <cstdint>
#include <set>

namespace o2::mch::raw::impl
{
template <typename T>
std::function<std::optional<o2::mch::raw::DsDetId>(o2::mch::raw::DsElecId)>
  mapperElec2Det(std::map<uint16_t, uint32_t> elec2det)
{
  return [elec2det](o2::mch::raw::DsElecId id) -> std::optional<o2::mch::raw::DsDetId> {
    auto it = elec2det.find(encode(id));
    if (it == elec2det.end()) {
      return std::nullopt;
    }
    return o2::mch::raw::decodeDsDetId(it->second);
  };
}

template <typename T>
std::function<std::optional<o2::mch::raw::DsElecId>(o2::mch::raw::DsDetId)>
  mapperDet2Elec(std::map<uint32_t, uint16_t> det2elec)
{
  return [det2elec](o2::mch::raw::DsDetId id) -> std::optional<o2::mch::raw::DsElecId> {
    auto it = det2elec.find(encode(id));
    if (it == det2elec.end()) {
      return std::nullopt;
    }
    return o2::mch::raw::decodeDsElecId(it->second);
  };
}

template <typename T>
std::function<std::optional<uint16_t>(uint16_t)>
  mapperSolar2Cru(std::map<uint16_t, uint16_t> solar2cru)
{
  return [solar2cru](uint16_t solarId) -> std::optional<uint16_t> {
    auto it = solar2cru.find(solarId);
    if (it == solar2cru.end()) {
      return std::nullopt;
    }
    return it->second;
  };
}

template <typename T>
std::function<std::set<uint16_t>(uint16_t)>
  mapperCru2Solar(std::map<uint16_t, std::set<uint16_t>> cru2solar)
{
  return [cru2solar](uint16_t cruId) -> std::set<uint16_t> {
    auto it = cru2solar.find(cruId);
    if (it == cru2solar.end()) {
      return {};
    }
    return it->second;
  };
}

} // namespace o2::mch::raw::impl

#endif
