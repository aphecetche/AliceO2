// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "Paginator.h"
#include "MCHRawCommon/RDHManip.h"
#include "Headers/RAWDataHeader.h"
#include "MCHRawEncoder/DataBlock.h"
#include <fmt/format.h>
#include <gsl/span>

extern std::ostream& operator<<(std::ostream&, const o2::header::RAWDataHeaderV4&);

namespace o2::mch::raw
{
/// A DataBlockSplitter takes an input buffer containing one
/// (PayloadHeader,Payload) block and creates an ouput buffer
/// with (rdh,payload) data block(s) of size pageSize,
/// i.e. it splits the payloads in pieces of size pageSize
///
/// If a payload is too small to fill a page, then the DataBlockSplitter must
/// fill the block with the relevant number of
/// padding bytes, so that (RDH,payload,paddingByte)
/// is the same size as other blocks.
///
/// @return the number of pages added to outBuffer
template <typename RDH>
using DataBlockSplitter = std::function<size_t(DataBlock block, std::vector<uint8_t>& outBuffer)>;

/// A StopPager adds one single page, composed of only
/// a RDH (i.e. with no payload), to the outBuffer.
/// That RDH must have the stop bit at 1.
///
/// @return the number of pages added to outBuffer
template <typename RDH>
using StopPager = std::function<void(PayloadHeader header, std::vector<uint8_t>& outBuffer, uint16_t pageCount)>;

template <typename RDH>
RDH createRDH(const PayloadHeader& header,
              size_t pageSize, uint16_t pageCnt = 0, uint16_t len = 0)
{
  RDH rdh;
  rdhOrbit(rdh, header.orbit);
  rdhBunchCrossing(rdh, header.bc);
  rdhFeeId(rdh, header.feeId);
  rdhOffsetToNext(rdh, pageSize);
  rdhPageCounter(rdh, pageCnt);
  rdhMemorySize(rdh, len + sizeof(RDH));
  return rdh;
}

template <typename RDH>
size_t createPaddedPage(DataBlock block,
                        std::vector<uint8_t>& outBuffer,
                        size_t pageSize,
                        uint8_t paddingByte)

{
  auto rdh = createRDH<RDH>(block.header, pageSize, 0, block.payload.size());
  appendRDH<RDH>(outBuffer, rdh);
  outBuffer.insert(outBuffer.end(), block.payload.begin(), block.payload.end());
  auto len = pageSize - block.payload.size() - sizeof(rdh);
  if (len) {
    std::fill_n(std::back_inserter(outBuffer), len, paddingByte);
  }
  return 1;
}

template <typename RDH>
size_t createSplitPages(DataBlock block, std::vector<uint8_t>& outBuffer,
                        size_t pageSize, uint8_t paddingByte)
{
  int payloadSize = block.header.payloadSize;
  int useableSize = pageSize - sizeof(RDH);
  int npages = std::ceil(1.0 * payloadSize / (useableSize));
  int inputPos{0};
  for (int i = 0; i < npages; i++) {
    auto len = std::min(payloadSize - inputPos, useableSize);
    auto ri = createRDH<RDH>(block.header, pageSize, i + 1, len);
    appendRDH<RDH>(outBuffer, ri);
    auto s = block.payload.subspan(inputPos, len);
    outBuffer.insert(outBuffer.end(), s.begin(), s.end());
    if (len < useableSize) {
      std::fill_n(std::back_inserter(outBuffer), useableSize - len, paddingByte);
    }
    inputPos += len;
  }
  return npages;
}

template <typename RDH>
DataBlockSplitter<RDH> createDataBlockSplitter(size_t pageSize,
                                               uint8_t paddingByte)
{
  return
    [pageSize, paddingByte](DataBlock block,
                            std::vector<uint8_t>& outBuffer) -> size_t {
      int payloadSize = block.header.payloadSize;
      int useableSize = pageSize - sizeof(block.header);
      if (payloadSize < useableSize) {
        return createPaddedPage<RDH>(block, outBuffer, pageSize, paddingByte);
      }
      return createSplitPages<RDH>(block, outBuffer, pageSize, paddingByte);
    };
}

template <typename RDH>
StopPager<RDH> createStopPager(size_t pageSize,
                               uint8_t paddingByte)
{
  return [pageSize, paddingByte](PayloadHeader header, std::vector<uint8_t>& outBuffer,
                                 uint16_t pageCount) {
    auto rdh = createRDH<RDH>(header, pageSize);
    rdhPageCounter(rdh, rdhPageCounter(rdh) + pageCount);
    rdhMemorySize(rdh, sizeof(rdh));
    rdhStop(rdh, 1);
    appendRDH<RDH>(outBuffer, rdh);
    auto len = pageSize - sizeof(rdh);
    std::fill_n(std::back_inserter(outBuffer), len, paddingByte);
  };
}

template <typename RDH>
size_t paginateBuffer(gsl::span<const uint8_t> buffer,
                      std::vector<uint8_t>& outBuffer,
                      size_t pageSize,
                      uint8_t paddingByte)
{
  auto payloadSplitter = createDataBlockSplitter<RDH>(pageSize, paddingByte);
  auto stopPager = createStopPager<RDH>(pageSize, paddingByte);

  size_t inputPos{0};
  size_t addedPages{0};

  while (inputPos < buffer.size() - sizeof(PayloadHeader)) {
    PayloadHeader header;
    memcpy(&header, &buffer[inputPos], sizeof(PayloadHeader));
    DataBlock block{header, buffer.subspan(inputPos + sizeof(header), header.payloadSize)};
    auto npages = payloadSplitter(block, outBuffer);
    if (npages) {
      stopPager(header, outBuffer, npages);
    }
    addedPages += npages;
    inputPos += header.payloadSize + sizeof(header);
  }
  return addedPages;
}

using header::RAWDataHeaderV4;

template size_t paginateBuffer<RAWDataHeaderV4>(gsl::span<const uint8_t> compactBuffer,
                                                std::vector<uint8_t>& outBuffer,
                                                size_t pageSize,
                                                uint8_t paddingByte);
} // namespace o2::mch::raw
