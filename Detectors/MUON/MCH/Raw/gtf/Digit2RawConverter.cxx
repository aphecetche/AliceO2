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

namespace o2::mch::raw
{
void showSize(const char* msg, uint64_t size)
{
  std::cout << msg << fmt::format(" buffer size is {} bytes ({:5.2f} MB)\n", size, 1.0 * size / 1024 / 1024);
}

#if 0
void writeIRs(gsl::span<o2::InteractionRecord> irs,
              const char* filename)
{
  std::ofstream os(filename);
  for (auto ir : irs) {
    os << ir << "\n";
  }
  os.close();
}
#endif

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

  for (auto p : digitsPerIR) {
    auto& currentIR = p.first;
    auto& digits = p.second;
    buffer.clear();
    encoder(digits, buffer, currentIR.orbit, currentIR.bc);
    showSize("after encoder", buffer.size());

    outBuffer.clear();
    size_t pageSize{0};
    normalizeBuffer<RDH>(buffer, outBuffer, pageSize);
    showSize("after normalizeBuffer", outBuffer.size());

    forEachRDH<RDH>(outBuffer, [](const RDH& rdh) {
      std::cout << fmt::format("ORB {:6d} BC {:4d}\n",
                               rdhOrbit(rdh), rdhBunchCrossing(rdh));
    });
    assignCruLink<RDH>(outBuffer, solar2cru);

    std::cout << "-------------------- SuperPaginating (to be written)\n";
    // FIXME: to be written...
    // superPaginate(buffer);
    out.write(reinterpret_cast<char*>(&outBuffer[0]), outBuffer.size());
    totalSize += outBuffer.size();
    n++;
    // if (n > 100) {
    //   break;
    // }
  }

  showSize(" totalSize", totalSize);
  std::cout << "\n";
}

template void digit2raw<o2::header::RAWDataHeaderV4>(
  const std::map<o2::InteractionRecord,
                 std::vector<o2::mch::Digit>>& digitsPerIR,
  DigitEncoder encoder,
  std::function<std::optional<CruLinkId>(uint16_t)> solar2cru,
  std::ostream& out);
} // namespace o2::mch::raw
