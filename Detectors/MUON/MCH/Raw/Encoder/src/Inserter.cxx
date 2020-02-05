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
#include "DetectorsRaw/HBFUtils.h"

namespace o2::mch::raw
{

/// Get the list of feeId referenced in the DataBlockHeaders of the buffer
std::set<uint16_t> getFeeIds(gsl::span<const uint8_t> buffer)
{
  std::set<uint16_t> feeIds;
  forEachDataBlockRef(
    buffer, [&feeIds](const DataBlockRef& ref) {
      feeIds.emplace(ref.block.header.feeId);
    });
  return feeIds;
}

void outputDataBlockRefs(gsl::span<const uint8_t> buffer, gsl::span<const DataBlockRef> dataBlockRefs,
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
                    gsl::span<const o2::InteractionRecord> emptyHBs)
{
  auto feeIds = getFeeIds(buffer);
  std::set<o2::InteractionRecord> allHBs(emptyHBs.begin(), emptyHBs.end());

  std::vector<DataBlockRef> dataBlockRefs;

  // get the list of (orbit,bc,feeId) triplets that are present
  // in the input buffer
  forEachDataBlockRef(
    buffer, [&dataBlockRefs, &feeIds, &allHBs](const DataBlockRef& ref) {
      dataBlockRefs.emplace_back(ref);
      allHBs.emplace(ref.block.header.bc, ref.block.header.orbit);
    });

  // compute the list of all the (orbit,bc,feeId) triplets
  // we need to have in the output buffer
  // (taking into account those already there plus the ones
  // from the emptyHBs list)
  for (auto hb : allHBs) {
    for (auto feeId : feeIds) {
      auto alreadyThere = std::find_if(dataBlockRefs.begin(),
                                       dataBlockRefs.end(), [hb, feeId](const DataBlockRef& h) {
                                         return h.block.header.bc == hb.bc &&
                                                h.block.header.orbit == hb.orbit &&
                                                h.block.header.feeId == feeId;
                                       });
      if (alreadyThere == dataBlockRefs.end()) {
        DataBlockHeader header{hb.orbit, hb.bc, feeId};
        dataBlockRefs.emplace_back(DataBlockRef{DataBlock{header, {}}});
      }
    }
  }

  // sort by feeId and by (orbit,bc)
  std::sort(dataBlockRefs.begin(), dataBlockRefs.end(), [](const DataBlockRef& a, const DataBlockRef& b) {
    return (a.block.header < b.block.header);
  });

  // write all hbframes (empty or not) to outBuffer
  outputDataBlockRefs(buffer, dataBlockRefs, outBuffer);
}

void insertHBs(gsl::span<const uint8_t> buffer,
               std::vector<uint8_t>& outBuffer,
               gsl::span<const o2::InteractionRecord> interactions)
{
  if (interactions.size() < 2) {
    return;
  }
  std::vector<o2::InteractionRecord> emptyHBFs;
  o2::raw::HBFUtils hbfutils(interactions[0]);
  hbfutils.fillHBIRvector(emptyHBFs, interactions[0], interactions[interactions.size() - 1]);
  insertEmptyHBs(buffer, outBuffer, emptyHBFs);
}

} // namespace o2::mch::raw
