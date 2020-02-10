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

namespace o2::mch::raw
{
void showSize(const char* msg, uint64_t size)
{
  std::cout << msg << fmt::format(" buffer size is {} bytes ({:5.2f} MB)\n", size, 1.0 * size / 1024 / 1024);
}

template <typename RDH>
void digit2raw(
  const std::map<o2::InteractionTimeRecord,
                 std::vector<o2::mch::Digit>>& digitsPerIR,
  DigitEncoder encoder,
  std::function<std::optional<CruLinkId>(uint16_t)> solar2cru,
  std::ostream& out)
{
  std::vector<o2::InteractionTimeRecord> interactions;
  interactions.reserve(digitsPerIR.size());
  for (auto& p : digitsPerIR) {
    interactions.push_back(p.first);
    std::cout << p.first << "\n";
  }

  std::vector<uint8_t> buffer;
  std::vector<uint8_t> outBuffer;
  uint64_t totalSize{0};

  int n{0};

  for (auto p : digitsPerIR) {
    buffer.clear();
    auto& digits = p.second;
    encoder(digits, buffer, p.first.orbit, p.first.bc);
    showSize("after encoder", buffer.size());

    outBuffer.clear();
    size_t pageSize{0};
    normalizeBuffer<RDH>(buffer, outBuffer, interactions, pageSize);
    showSize("after normalizeBuffer", outBuffer.size());

    assignCruLink<RDH>(outBuffer, solar2cru);

    std::cout << "-------------------- SuperPaginating (to be written)\n";
    // FIXME: to be written...
    // superPaginate(buffer);
    out.write(reinterpret_cast<char*>(&outBuffer[0]), outBuffer.size());
    totalSize += buffer.size();
    n++;
    if (n > 10) {
      break;
    }
  }

  std::cout << " collisions=" << interactions.size();
  showSize("totalSize", totalSize);
  std::cout << "\n";
}

template void digit2raw<o2::header::RAWDataHeaderV4>(
  const std::map<o2::InteractionTimeRecord,
                 std::vector<o2::mch::Digit>>& digitsPerIR,
  DigitEncoder encoder,
  std::function<std::optional<CruLinkId>(uint16_t)> solar2cru,
  std::ostream& out);
} // namespace o2::mch::raw
