// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

///
/// @author  Laurent Aphecetche

#define BOOST_TEST_MODULE Test MCHRaw bitset
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <iostream>
#include "MCHRaw/BitSet.h"
#include <vector>
#include <fmt/format.h>

using namespace o2::mch::raw;

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(bitset)

// Most of the tests (and their names) are adapted from github.com/mrrtf/sampa/pkg/bitset/bitset_test.go

BOOST_AUTO_TEST_CASE(TestCount)
{
  BitSet bs(10);
  BOOST_CHECK_NO_THROW(bs.set(1, true));
  BOOST_CHECK_NO_THROW(bs.set(2, true));
  BOOST_CHECK_NO_THROW(bs.set(3, false));
  BOOST_CHECK_NO_THROW(bs.set(9, true));
  BOOST_CHECK_EQUAL(bs.count(), 3);
}

BOOST_AUTO_TEST_CASE(TestAppend)
{
  BitSet bs(10);
  bs.append(true);
  bs.append(true);
  bs.append(false);
  bs.append(false);
  bs.append(true);
  BOOST_CHECK_EQUAL(bs.uint8(0, 5), 0x13);
}

BOOST_AUTO_TEST_CASE(TestPruneFirst)
{
  BitSet bs(9);
  BOOST_CHECK_NO_THROW(bs.setRangeFromString(0, 6, "1101011"));
  bs.pruneFirst(2);
  BOOST_CHECK_EQUAL(bs.stringLSBLeft(), "01011");
}

BOOST_AUTO_TEST_CASE(TestAny)
{
  BitSet bs(10);
  BOOST_CHECK_EQUAL(bs.any(), false);
  bs.set(2, true);
  BOOST_CHECK_EQUAL(bs.any(), true);
}

BOOST_AUTO_TEST_CASE(TestNew)
{
  BOOST_CHECK_THROW(BitSet a(-1), std::invalid_argument);
  BOOST_CHECK_THROW(BitSet a(0), std::invalid_argument);
  BOOST_CHECK_NO_THROW(BitSet a(100));
}

BOOST_AUTO_TEST_CASE(TestSet)
{
  BitSet bs(10);
  BOOST_CHECK_NO_THROW(bs.set(0, true));
  BOOST_CHECK_NO_THROW(bs.set(2, true));
  BOOST_CHECK_NO_THROW(bs.set(20, true));
  BOOST_CHECK_EQUAL(bs.size(), 32);
  BOOST_CHECK_EQUAL(bs.len(), 21);
  BOOST_CHECK_THROW(bs.set(-1, true), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(TestGet)
{
  BitSet bs(10);
  BOOST_CHECK_NO_THROW(bs.set(0, true));
  BOOST_CHECK_NO_THROW(bs.set(2, true));
  BOOST_CHECK_EQUAL(bs.get(0), true);
  BOOST_CHECK_EQUAL(bs.get(2), true);
  BOOST_CHECK_EQUAL(bs.get(1), false);
  BOOST_CHECK_THROW(bs.get(100), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(TestClear)
{
  BitSet bs(1);
  bs.set(24, true);
  BOOST_CHECK_EQUAL(bs.len(), 25);
  bs.clear();
  BOOST_CHECK_EQUAL(bs.len(), 0);
}

BOOST_AUTO_TEST_CASE(TestString)
{
  BitSet bs(8);
  bs.set(1, true);
  bs.set(3, true);
  bs.set(5, true);
  BOOST_CHECK_EQUAL(bs.stringLSBLeft(), "010101");
}

BOOST_AUTO_TEST_CASE(TestFromString)
{
  BOOST_CHECK_THROW(BitSet::fromString("00011x"), std::invalid_argument);
  BitSet bs = BitSet::fromString("01011011");
  BOOST_CHECK_EQUAL(bs.stringLSBLeft(), "01011011");
}

BOOST_AUTO_TEST_CASE(TestRangeFromString)
{
  BitSet bs = BitSet::fromString("110011");
  BOOST_CHECK_NO_THROW(bs.setRangeFromString(2, 3, "11"));
  BOOST_CHECK_EQUAL(bs.stringLSBLeft(), "111111");
  BOOST_CHECK_THROW(bs.setRangeFromString(2, 3, "x-"), std::invalid_argument);
  BOOST_CHECK_THROW(bs.setRangeFromString(4, 1, "101"), std::invalid_argument);
  BOOST_CHECK_THROW(bs.setRangeFromString(32, 38, "1100"), std::invalid_argument);
  BOOST_CHECK_NO_THROW(bs.setRangeFromString(32, 38, "1100110"));
  BOOST_CHECK_THROW(BitSet::fromString("abcd"), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(TestFromIntegers)
{
  uint64_t v = 0xF0F8FCFEFF3F3F1F;
  BitSet bs = BitSet::fromUint64(v);
  std::string s = "1111100011111100111111001111111101111111001111110001111100001111";
  BOOST_CHECK_EQUAL(bs.stringLSBLeft(), s);
  uint64_t x = bs.uint64(0, 63);
  BOOST_CHECK_EQUAL(x, v);

  bs = BitSet::fromUint8(0x13);
  BOOST_CHECK_EQUAL(bs.uint8(0, 7), 0x13);

  bs = BitSet::fromUint16(0x8000);
  BOOST_CHECK_EQUAL(bs.uint16(0, 15), 0x8000);

  bs = BitSet::fromUint32(0xF0008000);
  BOOST_CHECK_EQUAL(bs.uint32(0, 31), 0xF0008000);

  bs = BitSet(150);
  BOOST_CHECK_THROW(bs.setRangeFromUint8(0, 9, 0), std::invalid_argument);
  BOOST_CHECK_THROW(bs.setRangeFromUint16(9, 32, 0), std::invalid_argument);
  BOOST_CHECK_THROW(bs.setRangeFromUint32(24, 57, 0), std::invalid_argument);
  BOOST_CHECK_THROW(bs.setRangeFromUint64(56, 122, 0), std::invalid_argument);
  BOOST_CHECK_NO_THROW(bs.setRangeFromUint8(0, 7, 0xFF));
  BOOST_CHECK_EQUAL(bs.len(), 8);
}

BOOST_AUTO_TEST_CASE(TestRangeFromUint16)
{
  uint16_t v = 0xF0F8;

  auto bs = BitSet::fromUint16(v);
  bs.setRangeFromUint16(12, 14, 0);
  BOOST_CHECK_EQUAL(bs.uint16(0, 15), 0x80F8);
}

BOOST_AUTO_TEST_CASE(TestRangeFromUint32)
{
  uint32_t v = 0xF0F8FCFE;

  auto bs = BitSet::fromUint32(v);
  bs.setRangeFromUint32(28, 30, 0);
  BOOST_CHECK_EQUAL(bs.uint32(0, 31), 0x80F8FCFE);
}

BOOST_AUTO_TEST_CASE(TestRangeFromUint64)
{
  uint64_t v = 0xF0F8FCFEFF3F3F1F;

  auto bs = BitSet::fromUint64(v);
  bs.setRangeFromUint64(60, 62, 0);
  uint64_t expected = 0x80F8FCFEFF3F3F1F;
  BOOST_CHECK_EQUAL(bs.uint64(0, 63), expected);
}

BOOST_AUTO_TEST_CASE(TestRangeFromIntegers)
{
  BitSet bs(64);
  BOOST_CHECK_NO_THROW(bs.setRangeFromUint8(0, 5, 0x13));
  bs.set(8, true);
  BOOST_CHECK_NO_THROW(bs.setRangeFromUint8(20, 23, 0xF));
  BOOST_CHECK_NO_THROW(bs.setRangeFromUint32(29, 48, 0xAAAAA));
  BOOST_CHECK_EQUAL(bs.uint64(0, 63), 0x1555540F00113);
}

BOOST_AUTO_TEST_CASE(TestFromBytes)
{
  BitSet bs(150); // any number would work here, the size will be set by setFromBytes
  std::vector<uint8_t> bytes = {0xfe, 0x5a, 0x1e, 0xda};
  bs.setFromBytes(bytes);
  BOOST_CHECK_EQUAL(bs.uint32(0, 31), 0XDA1E5AFE);
  BOOST_CHECK_EQUAL(bs.size(), 32);
  BOOST_CHECK_EQUAL(bs.len(), 32);
}

BOOST_AUTO_TEST_CASE(TestIsEqual)
{
  auto b1 = BitSet::fromString("110011");
  auto b2 = BitSet::fromString("110011");
  BOOST_CHECK(b1 == b2);
  b2 = BitSet::fromString("1010");
  BOOST_CHECK(b1 != b2);
}

BOOST_AUTO_TEST_CASE(TestSub)
{
  auto bs = BitSet::fromString("110011");
  auto b = bs.subset(2, 4);
  BOOST_CHECK_EQUAL(b.stringLSBLeft(), "001");
  b.set(1, true);
  BOOST_CHECK_EQUAL(b.stringLSBLeft(), "011");
}

BOOST_AUTO_TEST_CASE(TestUint64)
{
  BitSet bs = BitSet::fromString("110011");
  auto v2 = bs.uint64(0, 5);
  BOOST_CHECK_EQUAL(v2, 51);
  v2 = bs.uint64(0, 1);
  BOOST_CHECK_EQUAL(v2, 3);

  uint64_t v = UINT64_C(0xFFFFFFFFFFFFFFFF);
  bs = BitSet::fromUint64(v);
  BOOST_CHECK_EQUAL(bs.stringLSBLeft(), "1111111111111111111111111111111111111111111111111111111111111111");
  auto x = bs.uint64(0, 63);
  BOOST_CHECK_EQUAL(x, v);
}

BOOST_AUTO_TEST_CASE(TestGrow)
{
  BitSet bs(32);
  BOOST_CHECK_EQUAL(bs.grow(16), false);
  BOOST_CHECK_THROW(bs.grow((BitSet::maxSize() + 1)), std::length_error);
  BOOST_CHECK_EQUAL(bs.grow(34), true);
}

BOOST_AUTO_TEST_CASE(TestUint16)
{
  uint16_t v = 0xFFFF;
  BitSet bs = BitSet::fromUint16(v);
  BOOST_CHECK_EQUAL(bs.stringLSBLeft(), "1111111111111111");
  auto x = bs.uint16(0, 15);
  BOOST_CHECK_EQUAL(x, v);
}

BOOST_AUTO_TEST_CASE(TestUint32)
{
  uint32_t v = 0xFFFFFFFF;
  BitSet bs = BitSet::fromUint32(v);
  BOOST_CHECK_EQUAL(bs.stringLSBLeft(), "11111111111111111111111111111111");
  auto x = bs.uint32(0, 31);
  BOOST_CHECK_EQUAL(x, v);
}

BOOST_AUTO_TEST_CASE(TestLast)
{
  auto bs = BitSet::fromString("1010110101111");
  BOOST_CHECK(bs.last(4) == BitSet::fromString("1111"));
  BOOST_CHECK(bs.last(6) == BitSet::fromString("101111"));
}

BOOST_AUTO_TEST_CASE(TestEmptyBitSet)
{
  BitSet bs(1);
  BOOST_CHECK_EQUAL(bs.stringLSBLeft(), "");
  BOOST_CHECK_EQUAL(bs.stringLSBRight(), "");
  BOOST_CHECK_EQUAL(bs.len(), 0);

  // emptyness is not a function of initial size
  bs = BitSet(64);
  BOOST_CHECK_EQUAL(bs.stringLSBLeft(), "");
  BOOST_CHECK_EQUAL(bs.stringLSBRight(), "");
  BOOST_CHECK_EQUAL(bs.len(), 0);
}

std::string bitNumberScale(int n, int nspaces, bool right2left)
{
  std::string line1;
  std::string line2;
  for (int i = 0; i < n; i++) {
    if (i > 0 && i % 10 == 0) {
      line1 += std::to_string(i / 10);
    } else {
      line1 += " ";
    }
    line2 += std::to_string(i % 10);
  }

  if (right2left) {
    std::reverse(begin(line1), end(line1));
    std::reverse(begin(line2), end(line2));
  }
  std::string spaces(nspaces, ' ');
  std::string rv;

  if (n > 10) {
    rv = spaces + line1 + "\n";
  }
  rv += spaces + line2;
  return rv;
}

BOOST_AUTO_TEST_CASE(TestAppendUint32)
{
  BitSet bs(32);
  auto C = UINT32_C(0XDA1E5AFE);

  bs.appendUint32(C);

  BOOST_CHECK_EQUAL(bs.len(), 32);
  BOOST_CHECK_EQUAL(bs.uint32(0, 31), C);

  std::cout << fmt::format("BS -> {0}\n", bs.stringLSBLeft());
  std::cout << bitNumberScale(32, 6, false) << "\n\n";

  std::cout << fmt::format("BS <- {0}\n", bs.stringLSBRight());
  std::cout << bitNumberScale(32, 6, true) << "\n\n";
}

BOOST_AUTO_TEST_CASE(TestAppendUint64)
{
  BitSet bs(64);

  auto C = UINT64_C(0x1555540f00113);
  bs.appendUint64(C, 64);

  BOOST_CHECK_EQUAL(bs.len(), 64);
  BOOST_CHECK_EQUAL(bs.uint64(0, 63), C);

  std::cout << fmt::format("BS -> {0}\n", bs.stringLSBLeft());
  std::cout << bitNumberScale(64, 6, false) << "\n\n";
  std::cout << fmt::format("BS <- {0}\n", bs.stringLSBRight());
  std::cout << bitNumberScale(64, 6, true) << "\n\n";
}

BOOST_AUTO_TEST_CASE(TestAppendUint8)
{
  BitSet bs(1);

  uint8_t a(42);

  BOOST_CHECK_THROW(bs.appendUint8(a, 9), std::invalid_argument);

  // append bits, letting appendUint8 compute the number
  // of actual bits to add
  BOOST_CHECK_NO_THROW(bs.appendUint8(a));

  BOOST_CHECK_EQUAL(bs.len(), 6);
  BOOST_CHECK_EQUAL(bs.uint8(0, 5), a);

  BOOST_CHECK_EQUAL(bs.stringLSBLeft(), "010101");
  BOOST_CHECK_EQUAL(bs.stringLSBRight(), "101010");

  bs.clear();
  // append bits, forcing 8 bits to be added
  bs.appendUint8(a, 8);
  BOOST_CHECK_EQUAL(bs.stringLSBLeft(), "01010100");
  BOOST_CHECK_EQUAL(bs.uint8(0, 7), a);

  bs = BitSet::fromString("111");
  bs.appendUint8(128, 8);

  BOOST_CHECK_EQUAL(bs.stringLSBLeft(), "11100000001");
}

// BenchmarkSetRangeFromUint8
// BenchmarkSetRangeFromUint32

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
