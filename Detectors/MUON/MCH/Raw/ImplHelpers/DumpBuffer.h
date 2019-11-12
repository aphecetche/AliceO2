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

#endif
