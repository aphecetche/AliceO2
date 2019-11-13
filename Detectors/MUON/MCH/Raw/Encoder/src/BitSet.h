// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_BITSET_H
#define O2_MCH_RAW_BITSET_H

#include <cstdint>
#include <gsl/span>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include "BitSet.h"
#include <algorithm>
#include <cmath>
#include <fmt/format.h>
#include <iostream>
#include <stdexcept>
#include "NofBits.h"
#include "MCHRawCommon/SampaHeader.h"

namespace o2
{
namespace mch
{
namespace raw
{

template <typename UNDERTYPE = uint8_t>
class BitSet
{

 public:
  BitSet();

  // construct a BitSet using a string composed of '0' (meaning
  // bit unset) and '1' (meaning bit set) characters
  // the length of the resulting bitset is that of the string.
  explicit BitSet(std::string_view s);

  ///@{
  // construct a bitset initialized with the x-bits value v
  explicit BitSet(uint8_t v, int n = -1);
  explicit BitSet(uint16_t v, int n = -1);
  explicit BitSet(uint32_t v, int n = -1);
  explicit BitSet(uint64_t v, int n = -1);
  ///@}

  // check equality
  bool operator==(const BitSet& rhs) const;
  bool operator!=(const BitSet& rhs) const;

  // any returns true if any of the bits is set
  bool any() const;

  // appends a bit at the current position (i.e. len-1)
  int append(bool val);

  ///@{
  // appends the n first bits from a x-bits word.
  // if n is < 0 it is computed for val (using log2(val)+1)
  // otherwise it should be >= log2(val)+1 and <=x
  // and the exact number of specified bits will be set
  // (to 0 or 1).
  // returns the number of bits actually added
  int append(uint8_t val, int n = -1);
  int append(uint16_t val, int n = -1);
  int append(uint32_t val, int n = -1);
  int append(uint64_t val, int n = -1);
  ///@}

  // count returns the number of bits set at 1
  int count() const;

  // sets all the bits to false (i.e. resets)
  void clear();

  // returns true if we hold not bit at all
  bool isEmpty() const { return len() == 0; }

  // sets the value of the bit at given pos
  void set(int pos, bool val);

  // gets the value of the bit at given pos
  bool get(int pos) const;

  // grows the BitSet so it can accomodate at least n bits. Returns true if size changed.
  bool grow(int n);

  // last returns a bitset containing the last n bits of the bitset
  // if there's not enough bits, throw an exception
  BitSet last(int n) const;

  // return the max number of bits this object can currently hold
  // it is a multiple of 8.
  int size() const { return mSize; }

  // return the max number of bits any bitset can hold
  static int maxSize() { return 2 * 8192; }

  // return the number of bits we are current holding
  int len() const { return mLen; }

  // return the max number of bits we've ever held
  int maxlen() const { return mMaxLen; }

  // pruneFirst removes the first n bits from the bitset
  void pruneFirst(int n);

  void setFast(int pos, bool val);

  void setFromBytes(gsl::span<uint8_t> bytes);

  // setRangeFromString populates the bits at indice [a,b] (inclusive range)
  // from the characters in the string: 0 to unset the bit (=false)
  // or 1 to set the bit (=true).
  // A string containing anything else than '0' or '1' is invalid and
  // triggers an exception
  void setRangeFromString(int a, int b, std::string_view s);

  ///@{
  // setRangeFromUint(a,b,uintX_t) populates the bits at indices [a,b] (inclusive range)
  // with the bits of value v. b-a must be <=X otherwise throws an exception
  void setRangeFromUint(int a, int b, uint8_t v);
  void setRangeFromUint(int a, int b, uint16_t v);
  void setRangeFromUint(int a, int b, uint32_t v);
  void setRangeFromUint(int a, int b, uint64_t v);
  ///@}

  // returns a textual representation of the BitSet
  // where the LSB is on the left
  std::string stringLSBLeft() const;

  // returns a textual representation of the BitSet
  // where the LSB is on the right
  std::string stringLSBRight() const;

  // subset returns a subset of the bitset.
  // subset is not a slice (i.e. not a reference, but a copy of the internals)
  // [a,b] inclusive
  BitSet subset(int a, int b) const;

  // uint8 converts the bit set into a 8-bits value, if possible.
  // if b is negative, it is set to the bitset length
  uint8_t uint8(int a, int b) const;

  // uint16 converts the bit set into a 16-bits value, if possible.
  // if b is negative, it is set to the bitset length
  uint16_t uint16(int a, int b) const;

  // uint32 converts the bit set into a 32-bits value, if possible.
  // if b is negative, it is set to the bitset length
  uint32_t uint32(int a, int b) const;

  // uint64 converts the bit set into a 64-bits value, if possible.
  // if b is negative, it is set to the bitset length
  uint64_t uint64(int a, int b) const;

 private:
  int mSize;   // max number of bits we can hold
  int mLen;    // actual number of bits we are holding
  int mMaxLen; // the max number of bits we've ever held
  std::vector<UNDERTYPE> mBytes;
};

template <typename UNDERTYPE>
int circularAppend(BitSet<UNDERTYPE>& bs, const BitSet<UNDERTYPE>& ringBuffer, int startBit, int n);

template <typename UNDERTYPE>
std::ostream& operator<<(std::ostream& os, const BitSet<UNDERTYPE>& bs);

template <typename UNDERTYPE>
std::string compactString(const BitSet<UNDERTYPE>& bs);

template <typename T>
void assertRange(int a, int b, T s)
{
  if (a > b || b - a >= sizeof(T) * 8) {
    throw std::invalid_argument(fmt::format("Range [a,b]=[{0},{1}] is incorrect (max range={2}])", a, b, sizeof(T) * 8));
  }
}

template <typename UNDERTYPE, typename T>
void setRangeFromIntegerFast(BitSet<UNDERTYPE>& bs, int a, int b, T v)
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

template <typename UNDERTYPE, typename T>
T uint(const BitSet<UNDERTYPE>& bs, int a, int b)
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

template <typename UNDERTYPE, typename T>
int appendT(BitSet<UNDERTYPE>& bs, T val, int n)
{
  if (n <= 0) {
    if (val > 0) {
      n = impl::nofBits(val);
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
}

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

template <typename UNDERTYPE>
BitSet<UNDERTYPE>::BitSet() : mSize(8), mLen(0), mMaxLen(0), mBytes(1)
{
}

template <typename UNDERTYPE>
BitSet<UNDERTYPE>::BitSet(uint8_t v, int n) : mSize(n > 0 ? n : 8), mLen(0), mBytes(1)
{
  if (n <= 0) {
    n = 8;
  }
  assertRange(1, n - 1, v);
  setRangeFromUint(0, n - 1, v);
}

template <typename UNDERTYPE>
BitSet<UNDERTYPE>::BitSet(uint16_t v, int n) : mSize(n > 0 ? n : 16), mLen(0), mBytes(2)
{
  if (n <= 0) {
    n = 16;
  }
  assertRange(1, n - 1, v);
  setRangeFromUint(0, n - 1, v);
}

template <typename UNDERTYPE>
BitSet<UNDERTYPE>::BitSet(uint32_t v, int n) : mSize(n > 0 ? n : 32), mLen(0), mBytes(4)
{
  if (n <= 0) {
    n = 32;
  }
  assertRange(1, n - 1, v);
  setRangeFromUint(0, n - 1, v);
}

template <typename UNDERTYPE>
BitSet<UNDERTYPE>::BitSet(uint64_t v, int n) : mSize(n > 0 ? n : 64), mLen(0), mBytes(8)
{
  if (n <= 0) {
    n = 64;
  }
  assertRange(1, n - 1, v);
  setRangeFromUint(0, n - 1, v);
}

template <typename UNDERTYPE>
BitSet<UNDERTYPE>::BitSet(std::string_view s) : mSize(8), mLen(0), mBytes(1)
{
  setRangeFromString(0, s.size() - 1, s);
}

template <typename UNDERTYPE>
bool BitSet<UNDERTYPE>::operator==(const BitSet& rhs) const
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

template <typename UNDERTYPE>
bool BitSet<UNDERTYPE>::operator!=(const BitSet& rhs) const
{
  return !(*this == rhs);
}

template <typename UNDERTYPE>
int BitSet<UNDERTYPE>::append(bool val)
{
  set(len(), val);
  return 1;
}

template <typename UNDERTYPE>
int BitSet<UNDERTYPE>::append(uint8_t val, int n)
{
  return appendT<UNDERTYPE, uint8_t>(*this, val, n);
}

template <typename UNDERTYPE>
int BitSet<UNDERTYPE>::append(uint16_t val, int n)
{
  return appendT<UNDERTYPE, uint16_t>(*this, val, n);
}

template <typename UNDERTYPE>
int BitSet<UNDERTYPE>::append(uint32_t val, int n)
{
  return appendT<UNDERTYPE, uint32_t>(*this, val, n);
}

template <typename UNDERTYPE>
int BitSet<UNDERTYPE>::append(uint64_t val, int n)
{
  return appendT<UNDERTYPE, uint64_t>(*this, val, n);
}

template <typename UNDERTYPE>
bool BitSet<UNDERTYPE>::any() const
{
  for (int i = 0; i < len(); i++) {
    if (get(i)) {
      return true;
    }
  }
  return false;
}

template <typename UNDERTYPE>
void BitSet<UNDERTYPE>::clear()
{
  std::fill(begin(mBytes), end(mBytes), 0);
  mLen = 0;
}

template <typename UNDERTYPE>
int BitSet<UNDERTYPE>::count() const
{
  int n{0};
  for (int i = 0; i < len(); i++) {
    if (get(i)) {
      n++;
    }
  }
  return n;
}

template <typename UNDERTYPE>
bool BitSet<UNDERTYPE>::get(int pos) const
{
  if (pos < 0 || pos >= size()) {
    throw std::out_of_range(fmt::format("pos {0} is out of bounds", pos));
  }
  constexpr size_t tsize = sizeof(UNDERTYPE) * 8;
  UNDERTYPE b = mBytes[pos / tsize];
  return ((b >> (pos % tsize)) & 1) == 1;
}

template <typename UNDERTYPE>
bool BitSet<UNDERTYPE>::grow(int n)
{
  if (n <= 0) {
    throw std::invalid_argument("n should be >= 0");
  }
  if (n < size()) {
    return false;
  }
  if (n > BitSet<UNDERTYPE>::maxSize()) {
    throw std::length_error(fmt::format("trying to allocate a bitset of more than {0} bytes", BitSet<UNDERTYPE>::maxSize()));
  }
  auto nbytes = mBytes.size();
  while (nbytes * 8 < n) {
    nbytes *= 2;
  }
  mBytes.resize(nbytes, 0);
  mSize = mBytes.size() * 8;
  return true;
}

template <typename UNDERTYPE>
BitSet<UNDERTYPE> BitSet<UNDERTYPE>::last(int n) const
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

template <typename UNDERTYPE>
void BitSet<UNDERTYPE>::pruneFirst(int n)
{
  if (len() < n) {
    throw std::invalid_argument(fmt::format("cannot prune {0} bits", n));
  }
  for (int i = 0; i < len() - n; i++) {
    set(i, get(i + n));
  }
  mLen -= n;
}

template <typename UNDERTYPE>
void BitSet<UNDERTYPE>::set(int pos, bool val)
{
  if (pos >= mSize) {
    grow(pos + 1);
  }
  if (pos < 0) {
    throw std::invalid_argument("pos should be > 0");
  }
  setFast(pos, val);
}

template <typename UNDERTYPE>
void BitSet<UNDERTYPE>::setFast(int pos, bool val)
{
  constexpr size_t tsize = sizeof(UNDERTYPE) * 8;
  UNDERTYPE ix = pos % tsize;
  UNDERTYPE& b = mBytes[pos / tsize];

  UNDERTYPE mask = 1 << ix;

  if (val) {
    b |= mask;
  } else {
    b &= ~mask;
  }

  if ((pos + 1) > mLen) {
    mLen = pos + 1;
    mMaxLen = std::max(mMaxLen, mLen);
  }
}

template <typename UNDERTYPE>
void BitSet<UNDERTYPE>::setFromBytes(gsl::span<uint8_t> bytes)
{
  if (mSize < bytes.size() * 8) {
    grow(bytes.size() * 8);
  }
  mBytes.clear();
  std::copy(bytes.begin(), bytes.end(), std::back_inserter(mBytes));
  mBytes.resize(bytes.size());
  mLen = mBytes.size() * 8;
  mMaxLen = std::max(mMaxLen, mLen);
  mSize = mLen;
}

template <typename UNDERTYPE>
void BitSet<UNDERTYPE>::setRangeFromString(int a, int b, std::string_view s)
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

template <typename UNDERTYPE>
void BitSet<UNDERTYPE>::setRangeFromUint(int a, int b, uint8_t v)
{
  setRangeFromIntegerFast<UNDERTYPE, uint8_t>(*this, a, b, v);
}

template <typename UNDERTYPE>
void BitSet<UNDERTYPE>::setRangeFromUint(int a, int b, uint16_t v)
{
  setRangeFromIntegerFast<UNDERTYPE, uint16_t>(*this, a, b, v);
}

template <typename UNDERTYPE>
void BitSet<UNDERTYPE>::setRangeFromUint(int a, int b, uint32_t v)
{
  setRangeFromIntegerFast<UNDERTYPE, uint32_t>(*this, a, b, v);
}

template <typename UNDERTYPE>
void BitSet<UNDERTYPE>::setRangeFromUint(int a, int b, uint64_t v)
{
  setRangeFromIntegerFast<UNDERTYPE, uint64_t>(*this, a, b, v);
}

template <typename UNDERTYPE>
std::string BitSet<UNDERTYPE>::stringLSBLeft() const
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

template <typename UNDERTYPE>
std::string BitSet<UNDERTYPE>::stringLSBRight() const
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

template <typename UNDERTYPE>
BitSet<UNDERTYPE> BitSet<UNDERTYPE>::subset(int a, int b) const
{
  if (a >= size() || b > size() || b < a) {
    auto msg = fmt::format("BitSet<UNDERTYPE>::subset : range [{},{}] is incorrect. size={}", a, b, size());
    throw std::invalid_argument(msg);
  }
  BitSet sub;
  sub.grow(b - a);
  for (int i = a; i <= b; i++) {
    sub.set(i - a, get(i));
  }
  return sub;
}

template <typename UNDERTYPE>
uint8_t BitSet<UNDERTYPE>::uint8(int a, int b) const
{
  return uint<UNDERTYPE, uint8_t>(*this, a, b);
}

template <typename UNDERTYPE>
uint16_t BitSet<UNDERTYPE>::uint16(int a, int b) const
{
  return uint<UNDERTYPE, uint16_t>(*this, a, b);
}

template <typename UNDERTYPE>
uint32_t BitSet<UNDERTYPE>::uint32(int a, int b) const
{
  return uint<UNDERTYPE, uint32_t>(*this, a, b);
}

template <typename UNDERTYPE>
uint64_t BitSet<UNDERTYPE>::uint64(int a, int b) const
{
  return uint<UNDERTYPE, uint64_t>(*this, a, b);
}

// append n bits to BitSet bs.
// those n bits are taken from ringBuffer, starting at ringBuffer[startBit]
// return the value of startBit to be used for a future call to this method
// so the sequence of ringBuffer is not lost.
template <typename UNDERTYPE>
int circularAppend(BitSet<UNDERTYPE>& bs, const BitSet<UNDERTYPE>& ringBuffer, int startBit, int n)
{
  int ix = startBit;

  while (n > 0) {
    bs.append(ringBuffer.get(ix));
    --n;
    ix++;
    ix %= ringBuffer.len();
  }
  return ix;
}

template <typename UNDERTYPE>
std::ostream& operator<<(std::ostream& os, const BitSet<UNDERTYPE>& bs)
{
  os << bs.stringLSBLeft();
  return os;
}

template <typename UNDERTYPE>
std::string compactString(const BitSet<UNDERTYPE>& bs)
{
  // replaces multiple sync patterns by nxSYNC
  static BitSet syncWord(sampaSync().uint64(), 50);

  if (bs.size() < 49) {
    return bs.stringLSBLeft();
  }
  std::string s;

  int i = 0;
  int nsync = 0;
  while (i + 49 < bs.len()) {
    bool sync{false};
    while (i + 49 < bs.len() && bs.subset(i, i + 49) == syncWord) {
      i += 50;
      nsync++;
      sync = true;
    }
    if (sync) {
      s += fmt::format("[{}SYNC]", nsync);
    } else {
      nsync = 0;
      s += bs.get(i) ? "1" : "0";
      i++;
    }
  }
  for (int j = i; j < bs.len(); j++) {
    s += bs.get(j) ? "1" : "0";
  }
  return s;
}
} // namespace raw
} // namespace mch
} // namespace o2
#endif
