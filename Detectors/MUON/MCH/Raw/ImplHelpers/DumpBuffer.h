// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_IMPL_HELPERS_DUMPBUFFER_H
#define O2_MCH_RAW_IMPL_HELPERS_DUMPBUFFER_H

#include <gsl/span>
#include <iostream>
#include <fmt/format.h>
#include <vector>
#include "MCHRawCommon/SampaHeader.h"

namespace o2::mch::raw::impl
{

template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 1>
void dumpBuffer(gsl::span<T> buffer)
{
  // dump a buffer of T

  int i{0};
  while (i < buffer.size()) {
    if (i % (16 / sizeof(T)) == 0) {
      std::cout << fmt::format("\n{:8d} : ", i * sizeof(T));
    }
    std::cout << fmt::format("{:0{}X} ", buffer[i], sizeof(T) * 2);
    i++;
  }
  std::cout << "\n";
}

uint64_t b8to64(const std::vector<uint8_t>& buffer, size_t i)
{
  return (static_cast<uint64_t>(buffer[i + 0])) |
         (static_cast<uint64_t>(buffer[i + 1]) << 8) |
         (static_cast<uint64_t>(buffer[i + 2]) << 16) |
         (static_cast<uint64_t>(buffer[i + 3]) << 24) |
         (static_cast<uint64_t>(buffer[i + 4]) << 32) |
         (static_cast<uint64_t>(buffer[i + 5]) << 40) |
         (static_cast<uint64_t>(buffer[i + 6]) << 48) |
         (static_cast<uint64_t>(buffer[i + 7]) << 56);
}

void dumpBuffer(const std::vector<uint8_t>& buffer)
{
  int i{0};
  while (i < buffer.size()) {
    if (i % 8 == 0) {
      std::cout << fmt::format("\n{:8d} : ", i);
    }
    uint64_t w = b8to64(buffer, i);
    i += 8;
    std::cout << fmt::format("{:016X} {:4d} {:4d} {:4d} {:4d} {:4d} ",
                             w,
                             (w & 0x3FF0000000000) >> 40,
                             (w & 0xFFC0000000) >> 30,
                             (w & 0x3FF00000) >> 20,
                             (w & 0xFFC00) >> 10,
                             (w & 0x3FF));
    SampaHeader h(w & 0x3FFFFFFFFFFFF);
    if (h.packetType() == SampaPacketType::Data) {
      std::cout << fmt::format(" n10 {:4d} chip {:2d} ch {:2d}",
                               h.nof10BitWords(), h.chipAddress(), h.channelAddress());
    }
  }
  std::cout << "\n";
}
} // namespace o2::mch::raw::impl
#endif
