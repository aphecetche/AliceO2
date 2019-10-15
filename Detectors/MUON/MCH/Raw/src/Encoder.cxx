// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHRaw/Encoder.h"
#include "MCHRaw/RAWDataHeader.h"
#include <fmt/format.h>

namespace o2
{
namespace mch
{
namespace raw
{

size_t addPadding(std::vector<uint32_t>& outBuffer,
                  const RAWDataHeader& rdh,
                  gsl::span<uint32_t> inBuffer,
                  size_t pageSize,
                  uint32_t paddingWord)

{
  auto rdhSize = sizeof(RAWDataHeader);
  auto ordh = rdh;
  ordh.offsetNextPacket = pageSize;
  appendRDH(outBuffer, ordh);
  std::copy(inBuffer.begin(), inBuffer.end(), std::back_inserter(outBuffer));
  auto len = pageSize / 4 - outBuffer.size();
  if (!len) {
    std::cout << "no padding to be put\n";
    return 0;
  }
  std::fill_n(std::back_inserter(outBuffer), len, paddingWord);
  return 1;
}

size_t addPages(std::vector<uint32_t>& outBuffer,
                const RAWDataHeader& rdh,
                gsl::span<uint32_t> inBuffer,
                size_t pageSize)
{
  int useableSize = pageSize - sizeof(rdh);
  int payloadSize = inBuffer.size() * sizeof(uint32_t);
  int npages = std::ceil(1.0 * payloadSize / (useableSize));
  int inputPos{0};
  for (int i = 0; i < npages; i++) {
    auto ri = rdh;
    ri.offsetNextPacket = pageSize;
    ri.pagesCounter = i + 1;
    ri.blockLength = pageSize - sizeof(rdh);
    ri.memorySize = pageSize;
    size_t len = 0;
    if (i == npages - 1) {
      ri.stopBit = 1;
      len = rdh.offsetNextPacket - (npages - 1) * useableSize - sizeof(rdh);
    } else {
      ri.stopBit = 0;
      len = pageSize - sizeof(rdh);
    }
    appendRDH(outBuffer, ri);
    auto s = inBuffer.subspan(inputPos, len / 4);
    std::copy(s.begin(), s.end(), std::back_inserter(outBuffer));
    inputPos += len / 4;
  }
  return npages;
}

size_t paginateBuffer(gsl::span<uint32_t> compactBuffer,
                      std::vector<uint32_t>& outBuffer,
                      size_t pageSize,
                      uint32_t paddingWord)
{
  size_t inputPos{0};
  size_t npages{0};

  while (inputPos < compactBuffer.size()) {
    auto rdh = createRDH(compactBuffer.subspan(inputPos));
    if (!isValid(rdh)) {
      std::cout << rdh << "\n";
      throw std::logic_error("got an invalid rdh");
    }
    auto payloadSize = rdhPayloadSize(rdh);
    inputPos += sizeof(rdh) / 4;
    const auto inBuffer = compactBuffer.subspan(inputPos, payloadSize / 4);
    if (rdh.offsetNextPacket < pageSize) {
      // payload not big enough to fill a complete page : we add padding words
      npages += addPadding(outBuffer, rdh, inBuffer, pageSize, paddingWord);
    } else {
      // payload bigger than one page : we create multiple pages
      npages += addPages(outBuffer, rdh, inBuffer, pageSize);
    }
    inputPos += rdh.offsetNextPacket / 4;
  }
  return npages;
}

} // namespace raw
} // namespace mch
} // namespace o2
