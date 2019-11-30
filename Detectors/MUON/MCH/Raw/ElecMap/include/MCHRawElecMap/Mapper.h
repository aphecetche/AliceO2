// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_ELECMAP_MAPPER_H
#define O2_MCH_RAW_ELECMAP_MAPPER_H

#include <functional>
#include <optional>
#include <set>
#include <stdexcept>
#include <cstdint>
#include "MCHRawElecMap/DsDetId.h"
#include "MCHRawElecMap/DsElecId.h"
#include "MCHMappingFactory/CreateSegmentation.h"
#include <fmt/format.h>

namespace o2::mch::raw
{

/**@name Primary mappers
    */
///@{

/// From (solarId,groupdId,index) to (deId,dsId)
template <typename T>
std::function<std::optional<DsDetId>(DsElecId id)> mapperElec2Det();

/// From cruId to { solarId }
template <typename T>
std::function<std::set<uint16_t>(uint16_t cruId)> mapperCru2Solar();

/// From (deId,dsId) to (solarId,groupId,index)
template <typename T>
std::function<std::optional<DsElecId>(DsDetId id)> mapperDet2Elec();

/// From solarId to cruId
template <typename T>
std::function<std::optional<uint16_t>(uint16_t solarId)> mapperSolar2Cru();
///@}

/// From deId to cruId.
///
/// This one can be specialized (for performance reasons)
/// or just implemented using the primary ones here.
///
template <typename T>
std::function<std::optional<uint16_t>(uint16_t deId)> mapperDe2Cru()
{
  // this is to be viewed as a default implementation
  // (i.e. not optimized)
  // feel free to specialize it for your T
  auto det2elec = mapperDet2Elec<T>();
  auto solar2cru = mapperSolar2Cru<T>();
  return [&, det2elec, solar2cru](uint16_t deId) -> std::optional<uint16_t> {
    std::set<uint16_t> deSolars;
    auto seg = o2::mch::mapping::segmentation(deId);
    std::optional<uint16_t> cruId;
    seg.forEachDualSampa([&deId, det2elec, solar2cru, &cruId](int dsId) {
      auto dselec = det2elec(DsDetId(deId, dsId));
      if (dselec.has_value()) {
        cruId = solar2cru(dselec->solarId());
        std::cout << fmt::format("DE {:4d} DS {:4d} SOLAR {:4d} CRU {}\n",
                                 deId, dsId, dselec->solarId(), (cruId.has_value() ? cruId.value() : -42));
        return;
      }
    });
    return cruId;
  };
}
///@}

} // namespace o2::mch::raw

#endif
