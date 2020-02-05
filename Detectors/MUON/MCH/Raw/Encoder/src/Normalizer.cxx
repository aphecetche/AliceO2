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
#include "Counter.h"
#include "Headers/RAWDataHeader.h"
#include "DetectorsRaw/HBFUtils.h"
#include "MCHRawCommon/RDHManip.h"

namespace o2::mch::raw
{
template <typename RDH>
void normalizeBuffer(gsl::span<const uint8_t> buffer,
                     std::vector<uint8_t>& outBuffer,
                     gsl::span<const o2::InteractionTimeRecord> interactions,
                     size_t pageSize,
                     uint8_t paddingByte)
{
  paginateBuffer<RDH>(buffer, outBuffer, pageSize, paddingByte);

#if 1
  o2::raw::HBFUtils hbfutils(interactions[0]);
  std::vector<o2::InteractionRecord> emptyHBFs;
  hbfutils.fillHBIRvector(emptyHBFs, interactions[0], interactions[interactions.size() - 1]);

  std::cout << "emptyHBFs\n";
  for (auto ir : emptyHBFs) {
    std::cout << ir << "\n";
  }
  insertEmptyHBs<RDH>(buffer, outBuffer, emptyHBFs);

  setPacketCounter<RDH>(outBuffer);

  forEachRDH<RDH>(outBuffer, [&hbfutils](RDH& rdh, gsl::span<uint8_t>::size_type offset) {
    o2::InteractionRecord rec(rdhBunchCrossing(rdh), rdhOrbit(rdh));
    rdhTriggerType(rdh, rdhTriggerType(rdh) | hbfutils.triggerType(rec));
  });
#endif
}

template void normalizeBuffer<o2::header::RAWDataHeaderV4>(gsl::span<const uint8_t> buffer,
                                                           std::vector<uint8_t>& outBuffer,
                                                           gsl::span<const o2::InteractionTimeRecord> interactions,
                                                           size_t pageSize,
                                                           uint8_t paddingByte);
} // namespace o2::mch::raw
