// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHRaw/BitSet.h"
#include <algorithm>
#include <cmath>
#include <fmt/format.h>
#include <iostream>
#include <stdexcept>

namespace o2::mch::raw
{

namespace
{

template <typename T>
void assertRange(int a, int b, T s)
{
  if (a > b || b - a >= sizeof(T) * 8) {
    throw std::invalid_argument(fmt::format("Range [a,b]=[{0},{1}] is incorrect (max range={2}])", a, b, sizeof(T) * 8));
  }
}

template <typename T>
void setRangeFromIntegerFast(BitSet& bs, int a, int b, T v)
{
  assertRange<T>(a, b, v);
  if (b > bs.size()) {
    bs.grow(b);
  }
  for (int i = 0; i <= b - a; i++) {
    T check = static_cast<T>(1) << i;
    bs.setFast(i + a, (v & check) != 0);
  }
}

template <typename T>
T uint(const BitSet& bs, int a, int b)
{
  if (b < 0) {
    b = bs.len() - 1;
  }
  if (a < 0 || b < 0 || (b - a) > (sizeof(T) * 8) || a >= bs.size() || b >= bs.size()) {
    throw std::out_of_range(fmt::format("Range [{0},{1}] out of range. bs.size()={2}", a, b, bs.size()));
  }
  T value{0};
  for (T i = a; i <= b; i++) {
    if (bs.get(i)) {
      value |= (static_cast<T>(1) << (i - a));
    }
  }
  return value;
}

template <typename T>
int appendT(BitSet& bs, T val, int n)
{
  if (n <= 0) {
    if (val > 0) {
      n = static_cast<int>(std::floor(log2(val)) + 1);
    } else {
      n = 1;
    }
  }

  if (n > sizeof(T) * 8) {
    throw std::invalid_argument(fmt::format("n={0} is invalid :  should be between 0 and {1}",
                                            n, sizeof(T)));
  }

  for (T i = 0; i < static_cast<T>(n); i++) {
    T p = static_cast<T>(1) << i;
    if ((val & p) == p) {
      bs.append(true);
    } else {
      bs.append(false);
    }
  }
  return n;
} // namespace

bool isValidString(std::string_view s)
{
  auto ones = std::count(begin(s), end(s), '1');
  auto zeros = std::count(begin(s), end(s), '0');
  return s.size() == ones + zeros;
}

void assertString(std::string_view s)
{
  if (!isValidString(s)) {
    throw std::invalid_argument(fmt::format("{0} is not a valid bitset string (should only get 0 and 1 in there", s));
  }
}

} // namespace

BitSet::BitSet() : mSize(8), mLen(0), mBytes(1)
{
}

BitSet::BitSet(uint8_t v) : mSize(8), mLen(0), mBytes(1)
{
  setRangeFromUint(0, 7, v);
}

BitSet::BitSet(uint16_t v) : mSize(16), mLen(0), mBytes(2)
{
  setRangeFromUint(0, 15, v);
}

BitSet::BitSet(uint32_t v) : mSize(32), mLen(0), mBytes(4)
{
  setRangeFromUint(0, 31, v);
}

BitSet::BitSet(uint64_t v) : mSize(64), mLen(0), mBytes(8)
{
  setRangeFromUint(0, 63, v);
}

BitSet::BitSet(std::string_view s) : mSize(8), mLen(0), mBytes(1)
{
  setRangeFromString(0, s.size() - 1, s);
}

bool BitSet::operator==(const BitSet& rhs) const
{
  if (len() != rhs.len()) {
    return false;
  }
  for (int i = 0; i < len(); i++) {
    if (get(i) != rhs.get(i)) {
      return false;
    }
  }
  return true;
}

bool BitSet::operator!=(const BitSet& rhs) const
{
  return !(*this == rhs);
}

int BitSet::append(bool val)
{
  set(len(), val);
  return 1;
}

int BitSet::append(uint8_t val, int n)
{
  return appendT<uint8_t>(*this, val, n);
}

int BitSet::append(uint16_t val, int n)
{
  return appendT<uint16_t>(*this, val, n);
}

int BitSet::append(uint32_t val, int n)
{
  return appendT<uint32_t>(*this, val, n);
}

int BitSet::append(uint64_t val, int n)
{
  return appendT<uint64_t>(*this, val, n);
}

bool BitSet::any() const
{
  for (int i = 0; i < len(); i++) {
    if (get(i)) {
      return true;
    }
  }
  return false;
}

void BitSet::clear()
{
  std::fill(begin(mBytes), end(mBytes), 0);
  mLen = 0;
}

int BitSet::count() const
{
  int n{0};
  for (int i = 0; i < len(); i++) {
    if (get(i)) {
      n++;
    }
  }
  return n;
}

bool BitSet::get(int pos) const
{
  if (pos < 0 || pos > size()) {
    throw std::out_of_range(fmt::format("pos {0} is out of bounds", pos));
  }
  uint8_t i = static_cast<uint8_t>(pos);
  auto b = mBytes[i / 8];
  i %= 8;
  return ((b >> i) & 1) == 1;
}

bool BitSet::grow(int n)
{
  if (n <= 0) {
    throw std::invalid_argument("n should be >= 0");
  }
  if (n < size()) {
    return false;
  }
  if (n > BitSet::maxSize()) {
    throw std::length_error(fmt::format("trying to allocate a bitset of more than {0} bytes", BitSet::maxSize()));
  }
  auto nbytes = mBytes.size();
  while (nbytes * 8 < n) {
    nbytes *= 2;
  }
  mBytes.resize(nbytes, 0);
  mSize = mBytes.size() * 8;
  return true;
}

BitSet BitSet::last(int n) const
{
  if (len() < n) {
    throw std::length_error(fmt::format("cannot get {0} bits out of a bitset of size {1}", n, len()));
  }
  auto subbs = subset(len() - n, len() - 1);
  if (subbs.len() != n) {
    throw std::logic_error("subset not of the expected len");
  }
  return subbs;
}

void BitSet::pruneFirst(int n)
{
  if (len() < n) {
    throw std::invalid_argument(fmt::format("cannot prune {0} bits", n));
  }
  for (int i = 0; i < len() - n; i++) {
    set(i, get(i + n));
  }
  for (int i = len() - n; i < len(); i++) {
    set(i, get(i + n));
  }
  mLen -= n;
}

void BitSet::set(int pos, bool val)
{
  if (pos >= mSize) {
    grow(pos + 1);
  }
  if (pos < 0) {
    throw std::invalid_argument("pos should be > 0");
  }
  setFast(pos, val);
}

void BitSet::setFast(int pos, bool val)
{
  uint8_t ix = pos % 8;
  uint8_t& b = mBytes[pos / 8];
  if (val) {
    b |= (static_cast<uint8_t>(1) << ix);
  } else {
    b &= ~(static_cast<uint8_t>(1) << ix);
  }
  if ((pos + 1) > mLen) {
    mLen = pos + 1;
  }
}

void BitSet::setFromBytes(gsl::span<uint8_t> bytes)
{
  if (mSize < bytes.size() * 8) {
    grow(bytes.size() * 8);
  }
  mBytes.clear();
  std::copy(bytes.begin(), bytes.end(), std::back_inserter(mBytes));
  mBytes.resize(bytes.size());
  mLen = mBytes.size() * 8;
  mSize = mLen;
}

void BitSet::setRangeFromString(int a, int b, std::string_view s)
{
  assertString(s);
  if (a > b) {
    throw std::invalid_argument(fmt::format("range [{0},{1}] is invalid : {0} > {1}", a, b));
  }
  if (b - a + 1 != s.size()) {
    throw std::invalid_argument(fmt::format("range [{0},{1}] is not the same size as string={2}", a, b, s));
  }
  if (a >= size() || b >= size()) {
    grow(b);
  }
  for (int i = 0; i < s.size(); i++) {
    set(i + a, s[i] == '1');
  }
}

void BitSet::setRangeFromUint(int a, int b, uint8_t v)
{
  setRangeFromIntegerFast<uint8_t>(*this, a, b, v);
}

void BitSet::setRangeFromUint(int a, int b, uint16_t v)
{
  setRangeFromIntegerFast<uint16_t>(*this, a, b, v);
}

void BitSet::setRangeFromUint(int a, int b, uint32_t v)
{
  setRangeFromIntegerFast<uint32_t>(*this, a, b, v);
}

void BitSet::setRangeFromUint(int a, int b, uint64_t v)
{
  setRangeFromIntegerFast<uint64_t>(*this, a, b, v);
}

std::string BitSet::stringLSBLeft() const
{
  std::string s{""};
  for (int i = 0; i < len(); i++) {
    if (get(i)) {
      s += "1";
    } else {
      s += "0";
    }
  }
  return s;
}

std::string BitSet::stringLSBRight() const
{
  std::string s{""};
  for (int i = len() - 1; i >= 0; i--) {
    if (get(i)) {
      s += "1";
    } else {
      s += "0";
    }
  }
  return s;
}

BitSet BitSet::subset(int a, int b) const
{
  if (a >= size() || b > size() || b < a) {
    throw std::invalid_argument(fmt::format("Range [{0},{1}] is incorrect", a, b));
  }
  BitSet sub;
  for (int i = a; i <= b; i++) {
    sub.set(i - a, get(i));
  }
  return sub;
}

uint8_t BitSet::uint8(int a, int b) const
{
  return uint<uint8_t>(*this, a, b);
}

uint16_t BitSet::uint16(int a, int b) const
{
  return uint<uint16_t>(*this, a, b);
}

uint32_t BitSet::uint32(int a, int b) const
{
  return uint<uint32_t>(*this, a, b);
}

uint64_t BitSet::uint64(int a, int b) const
{
  return uint<uint64_t>(*this, a, b);
}

} // namespace o2::mch::raw
