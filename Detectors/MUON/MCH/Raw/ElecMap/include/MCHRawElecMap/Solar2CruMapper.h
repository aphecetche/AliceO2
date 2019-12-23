// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_ELECMAP_SOLAR2CRU_MAPPER_H
#define O2_MCH_RAW_ELECMAP_SOLAR2CRU_MAPPER_H

#include <functional>
#include <cstdint>
#include <optional>

namespace o2::mch::raw
{
/// Mapper from solarId to cruId
using Solar2CruMapper = std::function<std::optional<uint16_t>(uint16_t solarId)>;
} // namespace o2::mch::raw

#endif
