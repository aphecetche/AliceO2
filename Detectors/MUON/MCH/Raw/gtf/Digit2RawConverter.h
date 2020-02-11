// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_ENCODER_DIGIT2RAW_CONVERTER_H
#define O2_MCH_RAW_ENCODER_DIGIT2RAW_CONVERTER_H

#include <map>
#include "CommonDataFormat/InteractionRecord.h"
#include <vector>
#include "MCHRawEncoder/DigitEncoder.h"
#include <ostream>
#include "MCHRawElecMap/CruLinkId.h"
#include <functional>
#include <optional>

namespace o2::mch::raw
{
template <typename RDH>
void digit2raw(const std::map<o2::InteractionRecord,
                              std::vector<o2::mch::Digit>>& digitsPerIR,
               DigitEncoder encoder,
               std::function<std::optional<CruLinkId>(uint16_t)> solar2cru,
               std::ostream& out);
}

#endif
