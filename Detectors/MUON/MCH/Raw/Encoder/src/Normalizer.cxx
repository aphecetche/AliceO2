// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHRawEncoder/Normalizer.h"

#include "Paginator.h"
#include "Inserter.h"
#include "RDHPacketCounterSetter.h"
#include "RDHTriggerTypeSetter.h"
#include "Headers/RAWDataHeader.h"
#include <iostream>

namespace o2::mch::raw
{

template <typename RDH>
void normalizeBuffer(gsl::span<const uint8_t> buffer,
                     std::vector<uint8_t>& outBuffer,
                     gsl::span<const o2::InteractionRecord> interactions,
                     size_t pageSize,
                     uint8_t paddingByte,
                     std::map<uint16_t, uint8_t>* previousPacketCounts)
{
  insertHBs(buffer, outBuffer, interactions);

  /// Turn the buffer into a real (RDH,payload) one
  /// There's one RDH at the start of each page
  std::vector<uint8_t> pages;
  paginateBuffer<RDH>(outBuffer, pages, pageSize, paddingByte);

  /// Ensure the packet counter of each link is correct
  /// FIXME: pass the counts as a parameter to normalizeBuffer
  if (previousPacketCounts) {
    setPacketCounter<RDH>(pages, *previousPacketCounts);
  } else {
    std::map<uint16_t, uint8_t> counts;
    setPacketCounter<RDH>(pages, counts);
  }

  setTriggerType<RDH>(pages, interactions);

  outBuffer.swap(pages);
}

template void normalizeBuffer<o2::header::RAWDataHeaderV4>(gsl::span<const uint8_t> buffer,
                                                           std::vector<uint8_t>& outBuffer,
                                                           gsl::span<const o2::InteractionRecord> interactions,
                                                           size_t pageSize,
                                                           uint8_t paddingByte,
                                                           std::map<uint16_t, uint8_t>* previousPacketCounts);
} // namespace o2::mch::raw
