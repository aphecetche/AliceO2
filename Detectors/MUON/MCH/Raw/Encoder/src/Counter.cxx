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
#include "LinkRange.h"

namespace o2::mch::raw
{
template <typename RDH>
void setPacketCounter(gsl::span<uint8_t> buffer)
{
  auto linkRanges = getLinkRanges<RDH>(buffer);
  for (auto lg : linkRanges) {
    uint8_t packetCount{0};
    for (auto l : lg.second) {
      forEachRDH<RDH>(buffer.subspan(l.start, l.size), [&packetCount](RDH& rdh, auto size) {
        rdhPacketCounter(rdh, packetCount);
        ++packetCount;
      });
    }
  }
}

// Provide only the specialization(s) we need
template void setPacketCounter<o2::header::RAWDataHeaderV4>(gsl::span<uint8_t> buffer);

} // namespace o2::mch::raw
