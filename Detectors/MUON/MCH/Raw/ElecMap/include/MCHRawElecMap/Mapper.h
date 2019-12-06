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
#include <array>
#include <gsl/span>

namespace o2::mch::raw
{

static std::array<int, 9> deIdsOfCH5R{504, 503, 502, 501, 500, 517, 516, 515, 514}; // from top to bottom
static std::array<int, 9> deIdsOfCH5L{505, 506, 507, 508, 509, 510, 511, 512, 513}; // from top to bottom
static std::array<int, 9> deIdsOfCH6R{604, 603, 602, 601, 600, 617, 616, 615, 614}; // from top to bottom
static std::array<int, 9> deIdsOfCH6L{605, 606, 607, 608, 609, 610, 611, 612, 613}; // from top to bottom

/**@name Primary mappers
    */
///@{

/// From (solarId,groupdId,index) to (deId,dsId)
/// use timestamp to specify a data taking period, use 0 to get the latest mapping
template <typename T>
std::function<std::optional<DsDetId>(DsElecId)> createElec2DetMapper(gsl::span<int> deIds,
                                                                     uint64_t timestamp = 0);

/// From (deId,dsId) to (solarId,groupId,index)
template <typename T>
std::function<std::optional<DsElecId>(DsDetId id)> createDet2ElecMapper(gsl::span<int> deIds);

/// From cruId to { solarId }
template <typename T>
std::function<std::set<uint16_t>(uint16_t cruId)> createCru2SolarMapper(gsl::span<int> deIds);

/// From solarId to cruId
template <typename T>
std::function<std::optional<uint16_t>(uint16_t solarId)> createSolar2CruMapper(gsl::span<int> deIds);
///@}

} // namespace o2::mch::raw

#endif
