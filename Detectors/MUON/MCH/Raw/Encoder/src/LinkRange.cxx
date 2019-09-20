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
#include "LinkRange.h"
#include "Headers/RAWDataHeader.h"

namespace o2::mch::raw
{
template <typename RDH>
std::map<int, std::vector<LinkRange>> getLinkRanges(gsl::span<uint8_t> buffer)
{
  std::map<int, std::vector<LinkRange>> linkRanges;
  constexpr auto rdhSize = static_cast<gsl::span<uint8_t>::size_type>(sizeof(RDH));
  auto bufSize = buffer.size();
  o2::mch::raw::forEachRDH<RDH>(buffer, [&linkRanges, rdhSize, bufSize](const RDH& rdh, gsl::span<uint8_t>::size_type offset) {
    uint32_t linkId = static_cast<uint32_t>(rdhLinkId(rdh)); // 0..23
    uint32_t linkUID = (static_cast<uint32_t>(rdhCruId(rdh)) << 16) | linkId;
    auto rsize = std::min(rdhSize + rdhPayloadSize(rdh), bufSize);
    auto& previousRanges = linkRanges[linkUID];
    if (previousRanges.empty()) {
      linkRanges[linkUID].emplace_back(LinkRange{offset, rsize});
    } else {
      auto& prev = previousRanges.back();
      if ((prev.start + prev.size) == offset) {
        // merge
        prev.size += rsize;
      } else {
        // add new one
        linkRanges[linkUID].emplace_back(LinkRange{offset, rsize});
      }
    }
  });

  return linkRanges;
}

void dumpLinkRanges(const std::map<int, std::vector<LinkRange>>& linkRanges)
{
  for (auto lg : linkRanges) {
    std::cout << fmt::format("LINK {} #ranges {}\n",
                             lg.first, lg.second.size());
    for (auto l : lg.second) {
      std::cout << fmt::format("   start {} size {}\n", l.start, l.size);
    }
  }
}

// Provide only the specialization(s) we need

template std::map<int, std::vector<LinkRange>> getLinkRanges<o2::header::RAWDataHeaderV4>(gsl::span<uint8_t> buffer);

} // namespace o2::mch::raw
