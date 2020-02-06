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
template <typename RDH>
void digit2raw(
  const std::map<o2::InteractionTimeRecord,
                 std::vector<o2::mch::Digit>>& digitsPerIR,
  DigitEncoder encoder,
  std::function<std::optional<CruLinkId>(uint16_t)> solar2cru,
  std::ostream& out)
{

  std::vector<uint8_t> buffer;
  for (auto p : digitsPerIR) {
    auto& digits = p.second;
    encoder(digits, buffer, p.first.orbit, p.first.bc);
  }

  std::vector<o2::InteractionTimeRecord> interactions;
  interactions.reserve(digitsPerIR.size());
  for (auto& p : digitsPerIR) {
    interactions.push_back(p.first);
  }
  std::vector<uint8_t> outBuffer;
  normalizeBuffer<RDH>(buffer, outBuffer, interactions);

  assignCruLink<RDH>(outBuffer, solar2cru);

  std::cout << "-------------------- SuperPaginating (to be written)\n";
  // FIXME: to be written...
  // superPaginate(buffer);
  out.write(reinterpret_cast<char*>(&outBuffer[0]), outBuffer.size());
  std::cout << fmt::format("output buffer is {} bytes ({:5.2f} MB)\n", outBuffer.size(), 1.0 * outBuffer.size() / 1024 / 1024);

  auto n = countRDHs<RDH>(buffer);

  std::cout << "n=" << n << " collisions=" << interactions.size() << "\n";
}
template void digit2raw<o2::header::RAWDataHeaderV4>(
  const std::map<o2::InteractionTimeRecord,
                 std::vector<o2::mch::Digit>>& digitsPerIR,
  DigitEncoder encoder,
  std::function<std::optional<CruLinkId>(uint16_t)> solar2cru,
  std::ostream& out);
} // namespace o2::mch::raw
