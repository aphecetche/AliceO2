// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#define BOOST_TEST_MODULE Test MCHRaw Hamming
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <fmt/format.h>
#include "MCHRaw/SampaHeader.h"
#include "MCHRaw/BitSet.h"
#include <bitset>

using namespace o2::mch::raw;

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(hamming)

void HammingDecode(unsigned int buffer[2], bool& error, bool& uncorrectable, bool fix_data)
//
// From Arild Velure code
//
{

  // header split
  bool parityreceived[6];
  bool data_in[43];
  bool overallparity;

  for (int i = 0; i < 6; i++)
    parityreceived[i] = (buffer[0] >> i) & 0x1;
  overallparity = (buffer[0] >> 6) & 0x1;
  //for (int i = 0; i < 43; i++)
  //  data_in[i] = (header_in >> (i + 7)) & 0x1;
  for (int i = 7; i < 30; i++)
    data_in[i - 7] = (buffer[0] >> i) & 0x1;
  for (int i = 30; i < 50; i++)
    data_in[i - 7] = (buffer[1] >> (i - 30)) & 0x1;

  //calculated values
  bool corrected_out[43];
  bool overallparitycalc = 0;
  bool overallparity_out = 0;
  bool paritycalc[6];
  bool paritycorreced_out[6];

  ////////////////////////////////////////////////////////////////////////////////////////////////
  // calculate parity
  // P0 24 values
  paritycalc[0] = data_in[0] ^ data_in[1] ^ data_in[3] ^ data_in[4] ^ data_in[6] ^
                  data_in[8] ^ data_in[10] ^ data_in[11] ^ data_in[13] ^ data_in[15] ^
                  data_in[17] ^ data_in[19] ^ data_in[21] ^ data_in[23] ^ data_in[25] ^
                  data_in[26] ^ data_in[28] ^ data_in[30] ^ data_in[32] ^ data_in[34] ^
                  data_in[36] ^ data_in[38] ^ data_in[40] ^ data_in[42];

  // P1 23 values
  paritycalc[1] = data_in[0] ^ data_in[2] ^ data_in[3] ^ data_in[5] ^ data_in[6] ^
                  data_in[9] ^ data_in[10] ^ data_in[12] ^ data_in[13] ^ data_in[16] ^
                  data_in[17] ^ data_in[20] ^ data_in[21] ^ data_in[24] ^ data_in[25] ^
                  data_in[27] ^ data_in[28] ^ data_in[31] ^ data_in[32] ^ data_in[35] ^
                  data_in[36] ^ data_in[39] ^ data_in[40];

  // P2 23 values
  paritycalc[2] = data_in[1] ^ data_in[2] ^ data_in[3] ^ data_in[7] ^ data_in[8] ^
                  data_in[9] ^ data_in[10] ^ data_in[14] ^ data_in[15] ^ data_in[16] ^
                  data_in[17] ^ data_in[22] ^ data_in[23] ^ data_in[24] ^ data_in[25] ^
                  data_in[29] ^ data_in[30] ^ data_in[31] ^ data_in[32] ^ data_in[37] ^
                  data_in[38] ^ data_in[39] ^ data_in[40];

  // P3 23 values
  paritycalc[3] = data_in[4] ^ data_in[5] ^ data_in[6] ^ data_in[7] ^ data_in[8] ^
                  data_in[9] ^ data_in[10] ^ data_in[18] ^ data_in[19] ^ data_in[20] ^
                  data_in[21] ^ data_in[22] ^ data_in[23] ^ data_in[24] ^ data_in[25] ^
                  data_in[33] ^ data_in[34] ^ data_in[35] ^ data_in[36] ^ data_in[37] ^
                  data_in[38] ^ data_in[39] ^ data_in[40];

  // P4 17 values
  paritycalc[4] = data_in[11] ^ data_in[12] ^ data_in[13] ^ data_in[14] ^ data_in[15] ^
                  data_in[16] ^ data_in[17] ^ data_in[18] ^ data_in[19] ^ data_in[20] ^
                  data_in[21] ^ data_in[22] ^ data_in[23] ^ data_in[24] ^ data_in[25] ^
                  data_in[41] ^ data_in[42];

  // P5 17 values
  paritycalc[5] = data_in[26] ^ data_in[27] ^ data_in[28] ^ data_in[29] ^ data_in[30] ^
                  data_in[31] ^ data_in[32] ^ data_in[33] ^ data_in[34] ^ data_in[35] ^
                  data_in[36] ^ data_in[37] ^ data_in[38] ^ data_in[39] ^ data_in[40] ^
                  data_in[41] ^ data_in[42];
  ////////////////////////////////////////////////////////////////////////////////////////////////

  //    uint8_t syndrome = 0;
  unsigned char syndrome = 0;

  for (int i = 0; i < 6; i++)
    syndrome |= (paritycalc[i] ^ parityreceived[i]) << i;

  bool data_parity_interleaved[64];
  bool syndromeerror;

  //data_parity_interleaved[0]          =  0;
  data_parity_interleaved[1] = parityreceived[0];
  data_parity_interleaved[2] = parityreceived[1];
  data_parity_interleaved[3] = data_in[0];
  data_parity_interleaved[4] = parityreceived[2];
  for (int i = 1; i <= 3; i++)
    data_parity_interleaved[i + 5 - 1] = data_in[i];
  data_parity_interleaved[8] = parityreceived[3];
  for (int i = 4; i <= 10; i++)
    data_parity_interleaved[i + 9 - 4] = data_in[i];
  data_parity_interleaved[16] = parityreceived[4];
  for (int i = 11; i <= 25; i++)
    data_parity_interleaved[i + 17 - 11] = data_in[i];
  data_parity_interleaved[32] = parityreceived[5];
  for (int i = 26; i <= 42; i++)
    data_parity_interleaved[i + 33 - 26] = data_in[i];
  //for (int i = 50; i <= 63; i++)
  //  data_parity_interleaved[i]        =  0;

  data_parity_interleaved[syndrome] = !data_parity_interleaved[syndrome]; // correct the interleaved

  paritycorreced_out[0] = data_parity_interleaved[1];
  paritycorreced_out[1] = data_parity_interleaved[2];
  corrected_out[0] = data_parity_interleaved[3];
  paritycorreced_out[2] = data_parity_interleaved[4];
  for (int i = 1; i <= 3; i++)
    corrected_out[i] = data_parity_interleaved[i + 5 - 1];
  paritycorreced_out[3] = data_parity_interleaved[8];
  for (int i = 4; i <= 10; i++)
    corrected_out[i] = data_parity_interleaved[i + 9 - 4];
  paritycorreced_out[4] = data_parity_interleaved[16];
  for (int i = 11; i <= 25; i++)
    corrected_out[i] = data_parity_interleaved[i + 17 - 11];
  paritycorreced_out[5] = data_parity_interleaved[32];
  for (int i = 26; i <= 42; i++)
    corrected_out[i] = data_parity_interleaved[i + 33 - 26];

  // now we have the "corrected" data -> update the flags

  bool wrongparity;
  for (int i = 0; i < 43; i++)
    overallparitycalc ^= data_in[i];
  for (int i = 0; i < 6; i++)
    overallparitycalc ^= parityreceived[i];
  syndromeerror = (syndrome > 0) ? 1 : 0; // error if syndrome larger than 0
  wrongparity = (overallparitycalc != overallparity);
  overallparity_out = !syndromeerror && wrongparity ? overallparitycalc : overallparity; // If error was in parity fix parity
  error = syndromeerror | wrongparity;
  uncorrectable = (syndromeerror && (!wrongparity));

  //header_out = 0;
  //for (int i = 0; i < 43; i++)
  //  header_out |= corrected_out[i] << (i + 7);
  //header_out |= overallparity_out << 6;
  //for (int i = 0; i < 6; i++)
  //  header_out |= paritycorreced_out[i] << i;
  if (fix_data) {
    for (int i = 0; i < 6; i++)
      buffer[0] = (buffer[0] & ~(1 << i)) | (paritycorreced_out[i] << i);
    buffer[0] = (buffer[0] & ~(1 << 6)) | (overallparity_out << 6);
    for (int i = 7; i < 30; i++)
      buffer[0] = (buffer[0] & ~(1 << i)) | (corrected_out[i - 7] << i);
    for (int i = 30; i < 50; i++)
      buffer[1] = (buffer[1] & ~(1 << (i - 30))) | (corrected_out[i - 7] << (i - 30));
  }
}

int parityEven(uint64_t v)
{
  // return the even parity of v
  std::bitset<64> bs(v);
  return (bs.count() + 1) % 2 == 0;
}

int parityOdd(uint64_t v)
{
  // return the odd parity of v
  std::bitset<64> bs(v);
  return (bs.count()) % 2 == 0;
}

template <size_t N>
std::string asString(std::bitset<N> bs)
{
  std::string s(bs.to_string());
  std::reverse(s.begin(), s.end());
  return s;
}

std::bitset<43> data43(uint64_t header)
{
  // extract the 43 data bits from a 50-bits header
  std::bitset<50> x50{header};
  std::bitset<43> x43;
  for (int i = 7; i < 50; i++) {
    x43[i - 7] = x50[i];
  }
  return x43;
}

/// compute parity of v, assuming it is 50 bits and
/// represents a Sampa header
/// (not using the existing header parity)
int headerParity(uint64_t v)
{
  std::bitset<50> h(v);
  h.reset(6); // reset existing parity bit
  return parityEven(h.to_ulong());
}

int partialOddParity(uint64_t value, int pos)
{
  // compute the odd parity of all the bits at position x
  // (where x & (2^pos)) are set
  int n{0};
  uint64_t one{1};
  std::array<int, 49> conv = {-1, -1, 0, -1, 1, 2, 3, -1, 4, 5, 6, 7, 8, 9, 10,
                              -1, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
                              22, 23, 24, 25, -1, 26, 27, 28, 29, 30, 31, 32,
                              33, 34, 35, 36, 37, 38, 39, 40, 41, 42};

  auto bs = data43(value);
  std::vector<int> tpos;
  std::vector<int> cpos;
  const uint64_t test{one << pos};
  for (uint64_t i = 0; i < 49; i++) {
    if (i & test) {
      tpos.push_back(i);
      cpos.push_back(conv[i]);
      if (conv[i] >= 0 && bs.test(conv[i])) {
        ++n;
      }
    }
  }
  std::cout << "bs=" << asString(bs) << " pos=" << pos << " tested:(" << tpos.size() << ") ";
  for (auto v : tpos) {
    std::cout << fmt::format("{:3d}", v) << " ";
  }
  std::cout << " n=" << n << "\n";
  std::cout << std::string(65, ' ');
  for (auto v : cpos) {
    std::cout << fmt::format("{:3d}", v) << " ";
  }
  std::cout << "\n";
  return (n + 1) % 2 == 0;
}

int hammingCode(uint64_t value)
{
  // value is assumed to be 50 bits, where the 43 data bits
  // are 7-49
  SampaHeader h(value);
  int p = headerParity(value);
  std::cout << h << " P=" << p << "\n";
  std::bitset<6> ham;

  for (int i = 0; i < 6; i++) {
    ham[i] = partialOddParity(value, i);
  }
  std::cout << h << " P=" << p << " p=" << h.headerParity() << " ham=" << asString(ham) << "\n";
  return p;
}

int CheckDataParity(uint64_t data)
{
  int parity, bit;
  parity = data & 0x1;
  for (int i = 1; i < 50; i++) {
    bit = (data >> i) & 0x1;
    parity = (parity || bit) && (!(parity && bit)); // XOR of all bits
  }
  return parity;
}

BOOST_AUTO_TEST_CASE(ParityEven)
{
  uint64_t one{1};
  std::vector<uint64_t> numbers = {1, 2, 3, 42, 1245, one << 49 | one << 32};

  for (auto n : numbers) {
    std::cout << fmt::format("P{}={} {}\n",
                             n, parityEven(n), CheckDataParity(n));
    BOOST_CHECK_EQUAL(parityEven(n), CheckDataParity(n) == 1);
  }

  BOOST_CHECK_EQUAL(headerParity(0x3722e80103208), 1); // 000100 P0
  BOOST_CHECK_EQUAL(headerParity(0x1722e8090322f), 1); // 111101 P0
  BOOST_CHECK_EQUAL(headerParity(0x1722e9f00327d), 0); // 101101 P1
}

BOOST_AUTO_TEST_CASE(PartialOddParity)
{
  BOOST_CHECK_EQUAL(partialOddParity(15, 0), 0);
  BOOST_CHECK_EQUAL(partialOddParity(15, 1), 0);
  BOOST_CHECK_EQUAL(partialOddParity(15, 3), 0);
}

BOOST_AUTO_TEST_CASE(TestHamming)
{
  // BOOST_CHECK_EQUAL(hammingCode(0x1FFFFFFFFFFC0), 0x8);
  BOOST_CHECK_EQUAL(hammingCode(0x3722e80103208), 0x8); // 000100 P0
  // BOOST_CHECK_EQUAL(hammingCode(0x1722e9f00327d), 0x2D); // 101101 P1
  // // BOOST_CHECK_EQUAL(hammingCode(0x1722e8090322f), 0x2F); // 111101 P0
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
