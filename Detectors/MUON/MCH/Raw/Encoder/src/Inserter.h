// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_ENCODER_INSERTER_H
#define O2_MCH_RAW_ENCODER_INSERTER_H

#include <gsl/span>
#include <cstdint>
#include <vector>
#include "CommonDataFormat/InteractionRecord.h"

namespace o2::mch::raw
{
// insertEmptyHBs will insert, for each link, empty DataBlockHeader
// corresponding to the interaction records in emptyHBs array, at the proper
// locations in the buffer
//
// That is, the buffer is supposed to contain only data for interaction records
// where there was some data in the detector and this function will add empty
// data (i.e. just DataBlockHeader) for all other heartbeats (defined in
// emptyHBs array).
//
// Note that the input buffer is untouched : the original data it contains,
// plus the empty HBs, will be appended to outBuffer instead

void insertEmptyHBs(gsl::span<const uint8_t> buffer,
                    std::vector<uint8_t>& outBuffer,
                    gsl::span<o2::InteractionRecord> emptyHBs);

} // namespace o2::mch::raw

#endif
