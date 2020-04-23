// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "Digit2RawConverter.h"
#include "MCHRawEncoder/Normalizer.h"
#include "MCHRawEncoder/CruLinkSetter.h"
#include "MCHRawEncoder/DataBlock.h"
#include <iostream>
#include <fmt/format.h>
#include "MCHRawCommon/RDHManip.h"
#include "Headers/RAWDataHeader.h"
#include <fstream>
#include "DetectorsRaw/HBFUtils.h"

namespace o2::header
{
extern std::ostream& operator<<(std::ostream&, const o2::header::RAWDataHeaderV4&);
}
namespace o2::mch::raw
{
class SuperPages
{
 public:
  SuperPages(std::ostream& out, size_t pageSize = 1024 * 1024) : mStream(out),
                                                                 mPageSize{pageSize} {}

  ~SuperPages()
  {
    flush();
  }

  void flush(std::vector<uint8_t>& page)
  {
    if (page.empty()) {
      return;
    }
    mStream.write(reinterpret_cast<char*>(&page[0]), page.size());
    mBytesWritten += page.size();
    page.clear();
    mFlush++;
  }

  void add(uint16_t feeId, gsl::span<uint8_t> buffer)
  {
    auto& page = mPages[feeId];
    if ((page.size() + buffer.size()) >= mPageSize) {
      flush(page);
    }
    page.insert(page.end(), buffer.begin(), buffer.end());
  }

  void flush()
  {
    for (auto& it : mPages) {
      flush(it.second);
    }
  }

  uint64_t bytesWritten() const { return mBytesWritten; }

 private:
  size_t mPageSize;
  std::ostream& mStream;
  std::map<uint16_t, std::vector<uint8_t>> mPages;
  size_t mFlush{0};
  uint64_t mBytesWritten{0};
};

template <typename RDH>
void digit2raw(
  const std::map<o2::InteractionRecord,
                 std::vector<o2::mch::Digit>>& digitsPerIR,
  const std::set<uint16_t>& feeIds,
  DigitEncoder encoder,
  std::function<std::optional<CruLinkId>(uint16_t)> solar2cru,
  std::ostream& out)
{
  std::vector<uint8_t> buffer;
  std::vector<uint8_t> outBuffer;

  o2::InteractionRecord firstIR = digitsPerIR.begin()->first;

  const uint16_t pageSize{0};

  BufferNormalizer<RDH> normalizer(firstIR, feeIds, pageSize);

  SuperPages superpages(out);

  for (auto p : digitsPerIR) {

    auto& currentIR = p.first;
    auto& digits = p.second;

    buffer.clear();
    encoder(digits, buffer, currentIR.orbit, currentIR.bc);

    outBuffer.clear();
    normalizer.normalize(buffer, outBuffer, currentIR);

    assignCruLink<RDH>(outBuffer, solar2cru);

    forEachRDH<RDH>(outBuffer, [&superpages, &outBuffer](RDH& rdh,
                                                         gsl::span<const uint8_t>::size_type offset) {
      gsl::span<uint8_t> page(&outBuffer[offset], rdhOffsetToNext(rdh));
      superpages.add(rdhFeeId(rdh), page);
    });
  }

  superpages.flush();
  auto size = superpages.bytesWritten();
  std::cout << fmt::format("totalSize is {} bytes ({:5.2f} MB)\n", size, 1.0 * size / 1024 / 1024);
}

template void digit2raw<o2::header::RAWDataHeaderV4>(
  const std::map<o2::InteractionRecord,
                 std::vector<o2::mch::Digit>>&,
  const std::set<uint16_t>&,
  DigitEncoder,
  std::function<std::optional<CruLinkId>(uint16_t)>,
  std::ostream&);
} // namespace o2::mch::raw
