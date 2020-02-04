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
#include "MCHRawCommon/RDHManip.h"
#include "Headers/RAWDataHeader.h"
#include <fmt/format.h>
#include <set>
#include <iostream>
#include "CommonConstants/Triggers.h"

namespace
{
using namespace o2::mch::raw;

struct HBRef {
  HBRef(uint32_t orbit, uint16_t bc, uint16_t _feeId)
    : ir(o2::InteractionRecord{bc, orbit}), feeId{_feeId}, empty{true}
  {
  }
  HBRef(uint32_t orbit, uint16_t bc, uint16_t _feeId,
        uint64_t _offset, uint16_t _size) : ir(o2::InteractionRecord{bc, orbit}), feeId(_feeId), offset(_offset), size(_size), empty{false}
  {
  }
  o2::InteractionRecord ir;
  uint16_t feeId{0};
  uint16_t size{0};
  uint64_t offset{0};
  bool empty;
};

std::ostream& operator<<(std::ostream& os, const HBRef& h)
{
  os << fmt::format("FEEID {:6d} IR ORB {:6d} BC {:6d}",
                    h.feeId, h.ir.orbit, h.ir.bc);
  if (!h.empty) {
    os << fmt::format(" OFF {:6d} SIZE {:4d}", h.offset, h.size);
  }
  return os;
}

/// Get the list of feeId referenced in the RDHs of the buffer
template <typename RDH>
std::set<uint16_t> getFeeIds(gsl::span<const uint8_t> buffer)
{
  std::set<uint16_t> feeIds;
  forEachRDH<RDH>(
    buffer, [&feeIds](const RDH& rdh) {
      feeIds.emplace(rdhFeeId(rdh));
    });
  return feeIds;
}

template <typename RDH>
RDH createEmptyRDH(const RDH& src)
{
  RDH rdh(src);
  rdhMemorySize(rdh, sizeof(RDH));
  rdhPageCounter(rdh, 0);
  rdhPacketCounter(rdh, 0);
  return rdh;
}

template <typename RDH>
RDH getRDH(gsl::span<const uint8_t> buffer,
           gsl::span<const HBRef> hbrefs,
           const HBRef& hbref)
{
  if (!hbref.empty) {
    throw std::invalid_argument("hbref should be of the empty kind");
  }
  auto h = std::find_if(hbrefs.begin(), hbrefs.end(),
                        [hbref](const HBRef& x) {
                          return x.ir == hbref.ir && x.feeId == hbref.feeId && !x.empty;
                        });
  if (h == hbrefs.end()) {
    // no matching for this exact combination, try with only feeId
    auto h = std::find_if(hbrefs.begin(), hbrefs.end(),
                          [hbref](const HBRef& x) {
                            return x.feeId == hbref.feeId && !x.empty;
                          });
    if (h == hbrefs.end()) {
      throw std::logic_error("should not be possible");
    }
    auto rdh = createRDH<RDH>(buffer.subspan(h->offset, sizeof(RDH)));
    // reset a few fields
    rdhOrbit(rdh, hbref.ir.orbit);
    rdhBunchCrossing(rdh, hbref.ir.bc);
    rdhMemorySize(rdh, sizeof(RDH));
    rdhTriggerType(rdh, o2::trigger::ORBIT | o2::trigger::HB);
    rdhPageCounter(rdh, 0);
    rdhPacketCounter(rdh, 0);
    return rdh;
  }
  return createRDH<RDH>(buffer.subspan(h->offset, sizeof(RDH)));
}

} // namespace

namespace o2::mch::raw
{
template <typename RDH>
void outputHBRefs(gsl::span<const uint8_t> buffer, gsl::span<const HBRef> hbrefs,
                  std::vector<uint8_t>& outBuffer)
{
  for (auto hbref : hbrefs) {
    std::cout << hbref << "\n";
    if (hbref.empty) {
      auto rdhModel = getRDH<RDH>(buffer, hbrefs, hbref);
      auto rdh = createEmptyRDH(rdhModel);
      appendRDH<RDH>(outBuffer, rdh);
      std::fill_n(outBuffer.end(), rdhOffsetToNext(rdh) - sizeof(RDH), 42);

    } else {
      auto block = buffer.subspan(hbref.offset, hbref.size);
      outBuffer.insert(end(outBuffer), block.begin(), block.end());
    }
  }
}

template <typename RDH>
void insertEmptyHBs(gsl::span<const uint8_t> buffer,
                    std::vector<uint8_t>& outBuffer,
                    gsl::span<o2::InteractionRecord> emptyHBs)
{
  auto feeIds = getFeeIds<RDH>(buffer);
  std::set<o2::InteractionRecord> allHBs(emptyHBs.begin(), emptyHBs.end());

  std::vector<HBRef> hbrefs;

  // get the list of (orbit,bc,feeId) triplets that are present
  // in the input buffer
  forEachRDH<RDH>(
    buffer, [&hbrefs, &feeIds, &allHBs](const RDH& rdh, uint64_t offset) {
      auto orbit = rdhOrbit(rdh);
      auto bc = rdhBunchCrossing(rdh);
      auto feeId = rdhFeeId(rdh);
      hbrefs.emplace_back(orbit, bc, feeId, offset, rdhOffsetToNext(rdh));
      allHBs.emplace(bc, orbit);
    });

  // compute the list of all the (orbit,bc,feeId) triplets
  // we need to have in the output buffer
  // (taking into account those already there plus the ones
  // from the emptyHBs list)
  for (auto hb : allHBs) {
    for (auto feeId : feeIds) {
      auto alreadyThere = std::find_if(hbrefs.begin(),
                                       hbrefs.end(), [hb, feeId](const HBRef& h) {
                                         return h.ir == hb && h.feeId == feeId;
                                       });
      if (alreadyThere == hbrefs.end()) {
        hbrefs.emplace_back(hb.orbit, hb.bc, feeId);
      }
    }
  }

  // sort by feeId and by (orbit,bc)
  std::sort(hbrefs.begin(), hbrefs.end(), [](const HBRef& a, const HBRef& b) {
    if (a.feeId < b.feeId) {
      return true;
    }
    if (a.feeId > b.feeId) {
      return false;
    }
    return a.ir < b.ir;
  });

  // write all hbframes (empty or not) to outBuffer
  outputHBRefs<RDH>(buffer, hbrefs, outBuffer);
}

using o2::header::RAWDataHeaderV4;
template void insertEmptyHBs<RAWDataHeaderV4>(gsl::span<const uint8_t> buffer,
                                              std::vector<uint8_t>& outBuffer,
                                              gsl::span<o2::InteractionRecord> emptyHBs);
} // namespace o2::mch::raw
