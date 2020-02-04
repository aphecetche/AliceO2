// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "CommonConstants/Triggers.h"
#include "MCHRawCommon/RDHManip.h"
#include <fmt/format.h>
#include "FeeIdRange.h"
#include "Headers/RAWDataHeader.h"

namespace o2::mch::raw
{
template <typename RDH>
std::map<int, std::vector<FeeIdRange>> getFeeIdRanges(gsl::span<const uint8_t> buffer)
{
  std::map<int, std::vector<FeeIdRange>> feeIdRanges;
  constexpr auto rdhSize = static_cast<gsl::span<const uint8_t>::size_type>(sizeof(RDH));
  auto bufSize = buffer.size();
  o2::mch::raw::forEachRDH<RDH>(buffer, [&feeIdRanges, rdhSize, bufSize](const RDH& rdh, gsl::span<const uint8_t>::size_type offset) {
    uint16_t feeId = rdhFeeId(rdh);
    auto rsize = std::min(rdhSize + rdhPayloadSize(rdh), bufSize);
    auto& previousRanges = feeIdRanges[feeId];
    if (previousRanges.empty()) {
      feeIdRanges[feeId].emplace_back(FeeIdRange{offset, rsize});
    } else {
      auto& prev = previousRanges.back();
      if ((prev.start + prev.size) == offset) {
        // merge
        prev.size += rsize;
      } else {
        // add new one
        feeIdRanges[feeId].emplace_back(FeeIdRange{offset, rsize});
      }
    }
  });

  return feeIdRanges;
}

void dumpFeeIdRanges(const std::map<int, std::vector<FeeIdRange>>& feeIdRanges)
{
  for (auto lg : feeIdRanges) {
    std::cout << fmt::format("FeeId {} #ranges {}\n",
                             lg.first, lg.second.size());
    for (auto l : lg.second) {
      std::cout << fmt::format("   start {} size {}\n", l.start, l.size);
    }
  }
}

// Provide only the specialization(s) we need

template std::map<int, std::vector<FeeIdRange>> getFeeIdRanges<o2::header::RAWDataHeaderV4>(gsl::span<const uint8_t> buffer);

} // namespace o2::mch::raw
