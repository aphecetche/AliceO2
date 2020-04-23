// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "Counter.h"
#include "MCHRawCommon/RDHManip.h"
#include "Headers/RAWDataHeader.h"
#include "FeeIdRange.h"
#include <map>

namespace o2::mch::raw
{
template <typename RDH>
void setPacketCounter(gsl::span<std::byte> buffer,
                      std::map<uint16_t, uint8_t>& initialCounters)
{
  auto feeRanges = getFeeIdRanges<RDH>(buffer);
  for (auto lg : feeRanges) {
    uint16_t feeId = lg.first;
    uint8_t& packetCount = initialCounters[feeId];
    for (auto l : lg.second) {
      forEachRDH<RDH>(buffer.subspan(l.start, l.size), [&packetCount](RDH& rdh, gsl::span<std::byte>::size_type) {
        rdhPacketCounter(rdh, packetCount);
        ++packetCount;
      });
    }
  }
}

// Provide only the specialization(s) we need
template void setPacketCounter<o2::header::RAWDataHeaderV4>(gsl::span<std::byte> buffer, std::map<uint16_t, uint8_t>& initialCounters);

} // namespace o2::mch::raw
