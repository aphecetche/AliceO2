// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_IMPL_HELPERS_MOVEBUFFER_H
#define O2_MCH_RAW_IMPL_HELPERS_MOVEBUFFER_H

#include <vector>
#include <iostream>

namespace o2::mch::raw::impl
{

/// Move the content of b64 to b8 and clears b64.
/// Returns the number of bytes moved into b8.

size_t moveBuffer(std::vector<uint64_t>& b64,
                  std::vector<uint8_t>& b8,
                  uint64_t prefix = 0)
{
  auto s8 = b8.size();
  b8.reserve(s8 + b64.size() / 8);
  for (auto& b : b64) {
    uint64_t g = b | prefix;
    b8.emplace_back(static_cast<uint8_t>((g & UINT64_C(0xFF))));
    b8.emplace_back(static_cast<uint8_t>((g & UINT64_C(0xFF00)) >> 8));
    b8.emplace_back(static_cast<uint8_t>((g & UINT64_C(0xFF0000)) >> 16));
    b8.emplace_back(static_cast<uint8_t>((g & UINT64_C(0xFF000000)) >> 24));
    b8.emplace_back(static_cast<uint8_t>((g & UINT64_C(0xFF00000000)) >> 32));
    b8.emplace_back(static_cast<uint8_t>((g & UINT64_C(0xFF0000000000)) >> 40));
    b8.emplace_back(static_cast<uint8_t>((g & UINT64_C(0xFF000000000000)) >> 48));
    b8.emplace_back(static_cast<uint8_t>((g & UINT64_C(0xFF00000000000000)) >> 56));
  }
  b64.clear();
  return b8.size() - s8;
}
} // namespace o2::mch::raw::impl

#endif
