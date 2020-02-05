// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHRawEncoder/DataBlock.h"
#include <fmt/format.h>

namespace o2::mch::raw
{
void appendHeader(std::vector<uint8_t>& outBuffer, PayloadHeader header)
{
  gsl::span<uint8_t> ph(reinterpret_cast<uint8_t*>(&header), sizeof(ph));
  outBuffer.insert(outBuffer.end(), ph.begin(), ph.end());
}

int forEachDataBlock(gsl::span<const uint8_t> buffer,
                     std::function<void(DataBlock block)> f)
{
  int index{0};
  int nheaders{0};
  PayloadHeader header;
  while (index < buffer.size()) {
    memcpy(&header, &buffer[index], sizeof(header));
    nheaders++;
    if (f) {
      f(DataBlock{header, buffer.subspan(index, header.payloadSize)});
    }
    index += header.payloadSize + sizeof(header);
  }
  return nheaders;
}

int countHeaders(gsl::span<uint8_t> buffer)
{
  return forEachDataBlock(buffer, nullptr);
}

std::ostream& operator<<(std::ostream& os, const PayloadHeader& header)
{
  os << fmt::format("ORB{:6d} BC{:4d} FEE{:4d} PAYLOADSIZE{:6d}",
                    header.orbit, header.bc, header.feeId, header.payloadSize);
  return os;
}
} // namespace o2::mch::raw
