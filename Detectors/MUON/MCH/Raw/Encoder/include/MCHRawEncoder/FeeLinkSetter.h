// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_ENCODER_FEELINK_SETTER_H
#define O2_MCH_RAW_ENCODER_FEELINK_SETTER_H

#include <gsl/span>
#include <functional>
#include <optional>
#include <cstdint>
#include "MCHRawElecMap/FeeLinkId.h"

namespace o2::mch::raw
{
template <typename RDH>
void assignFeeLink(gsl::span<std::byte> buffer,
                   std::function<std::optional<FeeLinkId>(uint16_t solarId)> solar2feelink);
}

#endif
