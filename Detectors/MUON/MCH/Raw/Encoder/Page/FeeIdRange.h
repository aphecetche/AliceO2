// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_ENCODER_FEEIDRANGE_H
#define O2_MCH_RAW_ENCODER_FEEIDRANGE_H

#include <gsl/span>
#include <cstdint>
#include <vector>
#include <map>

namespace o2::mch::raw
{
struct FeeIdRange {
  gsl::span<uint8_t>::size_type start;
  gsl::span<uint8_t>::size_type size;
};

// getFeeIdRanges return a map of (feeId->vector<FeeIdRange>)
// where FeeIdRange is a range of buffer's indices occupied by (rdh+payload)
// of a given feeId (aka (cru,link))
//
template <typename RDH>
std::map<int, std::vector<FeeIdRange>> getFeeIdRanges(gsl::span<const std::byte> buffer);

void dumpFeeIdRanges(const std::map<int, std::vector<FeeIdRange>>& feeIdRanges);
} // namespace o2::mch::raw

#endif
