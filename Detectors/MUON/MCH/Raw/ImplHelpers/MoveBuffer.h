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

/// Move the content of b64 to b8.
/// and clears b64.
size_t moveBuffer(std::vector<uint64_t>& b64,
                  std::vector<uint8_t>& b8,
                  uint64_t prefix = 0)
{
  constexpr uint64_t m = 0xFF;
  auto s = b8.size();
  for (auto& b : b64) {
    uint64_t g = b | prefix;
    for (uint64_t i = 0; i < 64; i += 8) {
      uint64_t w = m << i;
      b8.emplace_back(static_cast<uint8_t>((g & w) >> i));
    }
  }
  b64.clear();
  return b8.size() - s;
}

#endif
