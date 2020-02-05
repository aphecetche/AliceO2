// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_ENCODER_NORMALIZER_H
#define O2_MCH_RAW_ENCODER_NORMALIZER_H

#include <vector>
#include <gsl/span>
#include <cstdint>
#include "CommonDataFormat/InteractionRecord.h"

namespace o2::mch::raw
{

/// normalizeBuffer converts a buffer containing DataBlocks
/// (i.e. mch-specific (header,payload) blocks
/// corresponding to just the interactions in interactions array
/// into a "real" raw data buffer that :
///
/// - contains (RDH,payload) blocks
/// - is paginated in chunks of pageSize
/// - has RDH page and packet counters as they should be
/// - has proper "standalone" stop pages
/// - has RDH triggerType which is correct for HB and/or TF starts
/// - has small pages padded with a given paddingByte
///
/// (this last point is not strictly necessary but might help
/// distinguish quickly a simulated buffer wrt a real one)
///
template <typename RDH>
void normalizeBuffer(gsl::span<const uint8_t> buffer,
                     std::vector<uint8_t>& outBuffer,
                     gsl::span<const o2::InteractionTimeRecord> interactions,
                     size_t pageSize = 8192,
                     uint8_t paddingByte = 0x42);
} // namespace o2::mch::raw

#endif
