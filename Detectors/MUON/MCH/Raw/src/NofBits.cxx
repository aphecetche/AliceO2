// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "NofBits.h"
#include <cmath>
#include <fmt/format.h>

template <typename T>
int nofBitsT(T val)
{
  return static_cast<int>(std::floor(log2(1.0 * val)) + 1);
}

int nofBits(uint8_t val)
{
  return nofBitsT<uint8_t>(val);
}
int nofBits(uint16_t val)
{
  return nofBitsT<uint16_t>(val);
}
int nofBits(uint32_t val)
{
  return nofBitsT<uint32_t>(val);
}
int nofBits(uint64_t val)
{
  return nofBitsT<uint64_t>(val);
}

template <typename T>
void assertNofBitsT(std::string_view msg, T value, int n)
{
  // throws an exception if value is not contained within n bits
  if (static_cast<uint64_t>(value) >= (static_cast<uint64_t>(1) << n)) {
    throw std::invalid_argument(fmt::format("{} : 0x{:x} has {} bits, which is more than the {} allowed", msg, value, nofBits(value), n));
  }
}

void assertNofBits(std::string_view msg, uint8_t value, int n)
{
  assertNofBitsT<uint8_t>(msg, value, n);
}

void assertNofBits(std::string_view msg, uint16_t value, int n)
{
  assertNofBitsT<uint16_t>(msg, value, n);
}

void assertNofBits(std::string_view msg, uint32_t value, int n)
{
  assertNofBitsT<uint32_t>(msg, value, n);
}

void assertNofBits(std::string_view msg, uint64_t value, int n)
{
  assertNofBitsT<uint64_t>(msg, value, n);
}
