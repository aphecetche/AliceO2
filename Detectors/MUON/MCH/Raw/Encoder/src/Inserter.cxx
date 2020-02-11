// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "Inserter.h"
#include <fmt/format.h>
#include <set>
#include <iostream>
#include "CommonConstants/Triggers.h"
#include "MCHRawEncoder/DataBlock.h"

namespace o2::mch::raw
{

class DataBlockRefComp
{
 public:
  bool operator()(const DataBlockRef& a, const DataBlockRef& b) const
  {
    return a.block.header < b.block.header;
  }
};

void outputDataBlockRefs(gsl::span<const uint8_t> buffer, const std::set<DataBlockRef, DataBlockRefComp>& dataBlockRefs,
                         std::vector<uint8_t>& outBuffer)
{
  for (auto dataBlockRef : dataBlockRefs) {
    if (dataBlockRef.offset.has_value()) {
      auto block = buffer.subspan(dataBlockRef.offset.value(), dataBlockRef.block.size());
      outBuffer.insert(end(outBuffer), block.begin(), block.end());
    } else {
      appendDataBlockHeader(outBuffer, dataBlockRef.block.header);
    }
  }
}

// @return the first orbit in the buffer
uint32_t equalizeHBFPerFeeId(gsl::span<const uint8_t> buffer,
                             std::vector<uint8_t>& outBuffer)
{
  std::set<uint16_t> feeIds;
  std::set<o2::InteractionRecord> hbfs;

  std::set<DataBlockRef, DataBlockRefComp> dataBlockRefs;

  // get the list of (orbit,bc,feeId) triplets that are present
  // in the input buffer
  forEachDataBlockRef(
    buffer, [&dataBlockRefs, &feeIds, &hbfs](const DataBlockRef& ref) {
      dataBlockRefs.insert(ref);
      feeIds.insert(ref.block.header.feeId);
      hbfs.insert(o2::InteractionRecord{ref.block.header.bc, ref.block.header.orbit});
    });

  // compute the list of all the (orbit,bc,feeId) triplets
  // we need to have in the output buffer
  for (auto& hb : hbfs) {
    for (auto& feeId : feeIds) {
      DataBlockHeader header{hb.orbit, hb.bc, feeId};
      dataBlockRefs.insert(DataBlockRef{DataBlock{header, {}}});
    }
  }

  // write all hbframes (empty or not) to outBuffer
  outputDataBlockRefs(buffer, dataBlockRefs, outBuffer);

  return hbfs.begin()->orbit;
}
} // namespace o2::mch::raw
