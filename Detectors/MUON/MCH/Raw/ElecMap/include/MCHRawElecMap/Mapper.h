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
#include "MCHRawElecMap/FeeLinkId.h"
#include <fmt/format.h>
#include <array>
#include <gsl/span>

namespace o2::mch::raw
{

extern std::array<int, 156> deIdsForAllMCH;

/**@name Mapper templates.

  Those creator functions return functions that can do the mapping to/from 
  DsElecId to DsDetId and to/from FeeLinkId to solarId.
    */
///@{

using Elec2DetMapper = std::function<std::optional<DsDetId>(DsElecId)>;

/// From (solarId,groupdId,index) to (deId,dsId)
/// timestamp is foreseen to specify a data taking period (not used for the moment)
/// use 0 to get the latest mapping
template <typename T>
Elec2DetMapper createElec2DetMapper(gsl::span<int> deIds,
                                    uint64_t timestamp = 0);

using Det2ElecMapper = std::function<std::optional<DsElecId>(DsDetId id)>;
/// From (deId,dsId) to (solarId,groupId,index) for a given list of detection elements
template <typename T>
Det2ElecMapper createDet2ElecMapper(gsl::span<int> deIds);

/// From (deId,dsId) to (solarId,groupId,index) for all detection elements
template <typename T>
Det2ElecMapper createDet2ElecMapper()
{
  return createDet2ElecMapper<T>(deIdsForAllMCH);
}

/// From (feeId,linkId) to solarId
template <typename T>
std::function<std::optional<uint16_t>(FeeLinkId id)> createFeeLink2SolarMapper();

/// From solarId to (feeId,linkId)
template <typename T>
std::function<std::optional<FeeLinkId>(uint16_t solarId)> createSolar2FeeLinkMapper();
///@}

/**@name Actual mapper types.
    */
///@{

struct ElectronicMapperDummy {
};
struct ElectronicMapperGenerated {
};
///@}

extern std::array<int, 9> deIdsOfCH5R;
extern std::array<int, 9> deIdsOfCH5L;
extern std::array<int, 9> deIdsOfCH6R;
extern std::array<int, 9> deIdsOfCH6L;
extern std::array<int, 13> deIdsOfCH7R;
extern std::array<int, 13> deIdsOfCH7L;
extern std::array<int, 13> deIdsOfCH8R;
extern std::array<int, 13> deIdsOfCH8L;

std::function<std::set<int>(int deId)> createDualSampaMapper();

} // namespace o2::mch::raw

#endif
