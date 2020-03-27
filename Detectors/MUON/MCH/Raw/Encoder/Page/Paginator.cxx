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
/// (DataBlockHeader,Payload) block and creates an ouput buffer
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
using DataBlockSplitter = std::function<size_t(DataBlock block, std::vector<std::byte>& outBuffer)>;

/// A StopPager adds one single page, composed of only
/// a RDH (i.e. with no payload), to the outBuffer.
/// That RDH must have the stop bit at 1.
///
/// @return the number of pages added to outBuffer
template <typename RDH>
using StopPager = std::function<void(DataBlockHeader header, std::vector<std::byte>& outBuffer, uint16_t pageCount)>;

template <typename RDH>
RDH createRDH(const DataBlockHeader& header,
              size_t pageSize, uint16_t pageCnt = 0, uint16_t len = 0)
{
  RDH rdh;
  rdhOrbit(rdh, header.orbit);
  rdhBunchCrossing(rdh, header.bc);
  rdhFeeId(rdh, header.solarId);
  rdhOffsetToNext(rdh, pageSize);
  rdhPageCounter(rdh, pageCnt);
  rdhMemorySize(rdh, len + sizeof(RDH));
  return rdh;
}

template <typename RDH>
size_t createPaddedPage(DataBlock block,
                        std::vector<std::byte>& outBuffer,
                        size_t pageSize,
                        std::byte paddingByte)

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
size_t createSplitPages(DataBlock block, std::vector<std::byte>& outBuffer,
                        size_t pageSize, std::byte paddingByte)
{
  int payloadSize = block.header.payloadSize;
  int useableSize = pageSize - sizeof(RDH);
  int npages = std::ceil(1.0 * payloadSize / (useableSize));
  int inputPos{0};
  for (int i = 0; i < npages; i++) {
    auto len = std::min(payloadSize - inputPos, useableSize);
    auto ri = createRDH<RDH>(block.header, pageSize, i, len);
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
                                               std::byte paddingByte)
{
  if (pageSize == 0) {
    return [](DataBlock block, std::vector<std::byte>& outBuffer) {
      // do not paginate, just add a RDH (assuming payload is small enough
      // to fit in 2<<15 - 64)
      auto len = block.header.payloadSize;
      static constexpr uint64_t sizeLimit = 65472;
      if (len > sizeLimit) {
        throw std::invalid_argument("Payload too big to fit into an unsplit page...\n");
      }
      auto ri = createRDH<RDH>(block.header, block.header.payloadSize + sizeof(RDH), 0, len);
      appendRDH<RDH>(outBuffer, ri);
      auto s = block.payload.subspan(0, len);
      outBuffer.insert(outBuffer.end(), s.begin(), s.end());
      return 1;
    };
  }
  if (pageSize < sizeof(RDH)) {
    throw std::invalid_argument("cannot split with pageSize below rdh size");
  }
  return
    [pageSize, paddingByte](DataBlock block,
                            std::vector<std::byte>& outBuffer) -> size_t {
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
                               std::byte paddingByte)
{
  if (pageSize == 0) {
    return [pageSize, paddingByte](DataBlockHeader header, std::vector<std::byte>& outBuffer, uint16_t pageCount) {
      // append RDH with no payload, stop bit is set
      auto rdh = createRDH<RDH>(header, sizeof(RDH));
      rdhPageCounter(rdh, rdhPageCounter(rdh) + pageCount);
      rdhMemorySize(rdh, sizeof(rdh));
      rdhStop(rdh, 1);
      appendRDH<RDH>(outBuffer, rdh);
    };
  }
  if (pageSize < sizeof(RDH)) {
    throw std::invalid_argument("cannot split with pageSize below rdh size");
  }
  return [pageSize, paddingByte](DataBlockHeader header, std::vector<std::byte>& outBuffer, uint16_t pageCount) {
    // append RDH with padding payload to fill up a page
    // stop bit is set
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
size_t paginateBuffer(gsl::span<const std::byte> buffer,
                      std::vector<std::byte>& outBuffer,
                      size_t pageSize,
                      std::byte paddingByte)
{
  auto payloadSplitter = createDataBlockSplitter<RDH>(pageSize, paddingByte);
  auto stopPager = createStopPager<RDH>(pageSize, paddingByte);

  //auto solar2feelinkid;

  constexpr auto headerSize = sizeof(DataBlockHeader);

  if (buffer.size() < headerSize) {
    return 0;
  }

  size_t inputPos{0};
  size_t addedPages{0};

  while (inputPos < buffer.size()) {
    DataBlockHeader header;
    memcpy(&header, &buffer[inputPos], headerSize);
    inputPos += headerSize;
    DataBlock block{header, buffer.subspan(inputPos, header.payloadSize)};
    auto npages = payloadSplitter(block, outBuffer);
    if (npages) {
      stopPager(header, outBuffer, npages);
    }
    addedPages += npages;
    inputPos += header.payloadSize;
  }
  return addedPages;
}

using header::RAWDataHeaderV4;

template size_t paginateBuffer<RAWDataHeaderV4>(gsl::span<const std::byte> compactBuffer,
                                                std::vector<std::byte>& outBuffer,
                                                size_t pageSize,
                                                std::byte paddingByte);
} // namespace o2::mch::raw
