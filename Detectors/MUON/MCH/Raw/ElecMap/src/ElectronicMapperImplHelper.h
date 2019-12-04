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
} // namespace o2::mch::raw::impl

#endif
