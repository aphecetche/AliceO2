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
void showSize(const char* msg, uint64_t size)
{
  std::cout << msg << fmt::format(" buffer size is {} bytes ({:5.2f} MB)\n", size, 1.0 * size / 1024 / 1024);
}

void assertBufferIsNonEmpty(std::vector<uint8_t>& buffer,
                            const o2::InteractionRecord& ir, uint16_t feeId)
{
  appendDataBlockHeader(buffer, DataBlockHeader{ir.orbit, ir.bc, feeId, 0});
}

uint16_t getOneFeeId(DigitEncoder encoder)
{
  // use the encoder to get a valid feeid. any feeid will do.

  // create a vector of one single fake digit from first pad of DE 100
  std::vector<o2::mch::Digit> digits{o2::mch::Digit(0, 100, 0, 0)};
  // encode that single digit vector
  std::vector<uint8_t> buffer;
  encoder(digits, buffer, 0, 0);
  if (buffer.empty()) {
    throw std::logic_error("buffer should not be empty at this stage");
  }
  // extract feeId from the first header on the encoded buffer
  DataBlockHeader header;
  memcpy(&header, &buffer[0], sizeof(header));
  return header.feeId;
}

template <typename RDH>
void digit2raw(
  const std::map<o2::InteractionRecord,
                 std::vector<o2::mch::Digit>>& digitsPerIR,
  DigitEncoder encoder,
  std::function<std::optional<CruLinkId>(uint16_t)> solar2cru,
  std::ostream& out)
{
  std::vector<uint8_t> buffer;
  std::vector<uint8_t> outBuffer;
  uint64_t totalSize{0};

  int n{0};

  o2::InteractionRecord firstIR = digitsPerIR.begin()->first;
  const uint16_t defaultFeeId = getOneFeeId(encoder);

  const uint16_t pageSize{0};

  BufferNormalizer<RDH> normalizer(firstIR, pageSize);

  for (auto p : digitsPerIR) {
    auto& currentIR = p.first;
    std::cout << "AAA " << currentIR << "\n";
    auto& digits = p.second;
    buffer.clear();
    encoder(digits, buffer, currentIR.orbit, currentIR.bc);
    showSize("after encoder", buffer.size());

    // even if no digit we must have a non-empty buffer
    // for each HBF... create a "fake" one
    assertBufferIsNonEmpty(buffer, currentIR, defaultFeeId);

    outBuffer.clear();
    normalizer.normalize(buffer, outBuffer);
    showSize("after normalizeBuffer", outBuffer.size());

    assignCruLink<RDH>(outBuffer, solar2cru);

    std::cout << "-------------------- SuperPaginating (to be written)\n";
    // FIXME: to be written...
    // superPaginate(buffer);
    out.write(reinterpret_cast<char*>(&outBuffer[0]), outBuffer.size());
    totalSize += outBuffer.size();
    n++;
  }

  showSize(" totalSize", totalSize);
  std::cout << "\n";
}

template void digit2raw<o2::header::RAWDataHeaderV4>(
  const std::map<o2::InteractionRecord,
                 std::vector<o2::mch::Digit>>&,
  DigitEncoder,
  std::function<std::optional<CruLinkId>(uint16_t)>,
  std::ostream&);
} // namespace o2::mch::raw
