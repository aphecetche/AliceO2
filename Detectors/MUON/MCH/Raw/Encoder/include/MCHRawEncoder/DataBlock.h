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

namespace o2::mch::raw
{
struct PayloadHeader {
  uint32_t orbit;
  uint16_t bc;
  uint16_t feeId;
  uint64_t payloadSize;
};

struct DataBlock {
  PayloadHeader header;
  gsl::span<const uint8_t> payload;
};

void appendHeader(std::vector<uint8_t>& outBuffer, PayloadHeader header);

int forEachDataBlock(gsl::span<const uint8_t> buffer,
                     std::function<void(DataBlock block)> f);

int countHeaders(gsl::span<uint8_t> buffer);

std::ostream& operator<<(std::ostream& os, const PayloadHeader& header);
} // namespace o2::mch::raw

#endif
