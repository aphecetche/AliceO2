// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_ENCODER_DATABLOCK_H
#define O2_MCH_RAW_ENCODER_DATABLOCK_H

#include <cstdint>
#include <vector>
#include <gsl/span>
#include <functional>
#include <iostream>
#include <optional>

namespace o2::mch::raw
{
struct DataBlockHeader {
  uint32_t orbit;
  uint16_t bc;
  uint16_t solarId;
  uint64_t payloadSize;
};

struct DataBlock {
  DataBlockHeader header;
  gsl::span<const std::byte> payload;
  uint64_t size() const
  {
    return sizeof(header) + payload.size();
  }
};

struct DataBlockRef {
  DataBlock block;
  std::optional<uint64_t> offset;
};

void appendDataBlockHeader(std::vector<std::byte>& outBuffer, DataBlockHeader header);

int forEachDataBlockRef(gsl::span<const std::byte> buffer,
                        std::function<void(DataBlockRef blockRef)> f);

int countHeaders(gsl::span<const std::byte> buffer);

std::ostream& operator<<(std::ostream& os, const DataBlockHeader& header);
std::ostream& operator<<(std::ostream& os, const DataBlockRef& ref);
std::ostream& operator<<(std::ostream& os, const DataBlock& block);

bool operator<(const DataBlockHeader& a, const DataBlockHeader& b);
} // namespace o2::mch::raw

#endif
