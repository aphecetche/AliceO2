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
#include <fmt/format.h>
#include "MCHRawEncoder/DataBlock.h"

namespace
{
void showSize(const char* msg, gsl::span<const uint8_t> buffer)
{
  std::cout << msg << fmt::format(" buffer size is {} bytes ({:5.2f} MB)\n", buffer.size(), 1.0 * buffer.size() / 1024 / 1024);
}

} // namespace

namespace o2::mch::raw
{
template <typename RDH>
void setHBAndTFBits(gsl::span<uint8_t> pages, o2::InteractionRecord firstIR)
{
  o2::raw::HBFUtils hbfutils(firstIR);
  /// Ensure the triggerType of each RDH is correctly set
  forEachRDH<RDH>(pages, [&hbfutils](RDH& rdh, gsl::span<uint8_t>::size_type offset) {
    o2::InteractionRecord rec(rdhBunchCrossing(rdh), rdhOrbit(rdh));
    rdhTriggerType(rdh, rdhTriggerType(rdh) | hbfutils.triggerType(rec));
  });
}

template <typename RDH>
BufferNormalizer<RDH>::BufferNormalizer(o2::InteractionRecord firstIR,
                                        uint16_t pageSize,
                                        uint8_t paddingByte) : mFirstIR{firstIR},
                                                               mPageSize{pageSize},
                                                               mPaddingByte{paddingByte}
{
}

template <typename RDH>
void BufferNormalizer<RDH>::normalize(gsl::span<const uint8_t> buffer,
                                      std::vector<uint8_t>& outBuffer)
{
  if (buffer.empty()) {
    throw std::logic_error("Should not be called on an empty buffer");
  }
  /// ensure all feeIds have the same number of HBF
  equalizeHBFPerFeeId(buffer, outBuffer);

  /// Turn the buffer into a real (RDH,payload) one
  /// There's one RDH at the start of each page
  std::vector<uint8_t> pages;
  paginateBuffer<RDH>(outBuffer, pages, mPageSize, mPaddingByte);

  /// Ensure the packet counter of each link is correct
  setPacketCounter<RDH>(pages, mFeeIdPacketCounters);

  setHBAndTFBits<RDH>(pages, mFirstIR);

  outBuffer.swap(pages);
}

template class BufferNormalizer<o2::header::RAWDataHeaderV4>;
} // namespace o2::mch::raw
