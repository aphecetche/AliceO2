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
  paritycalc[0] = data_in[0] ^ data_in[1] ^ data_in[3] ^ data_in[4] ^ data_in[6] ^
                  data_in[8] ^ data_in[10] ^ data_in[11] ^ data_in[13] ^ data_in[15] ^
                  data_in[17] ^ data_in[19] ^ data_in[21] ^ data_in[23] ^ data_in[25] ^
                  data_in[26] ^ data_in[28] ^ data_in[30] ^ data_in[32] ^ data_in[34] ^
                  data_in[36] ^ data_in[38] ^ data_in[40] ^ data_in[42];

  paritycalc[1] = data_in[0] ^ data_in[2] ^ data_in[3] ^ data_in[5] ^ data_in[6] ^
                  data_in[9] ^ data_in[10] ^ data_in[12] ^ data_in[13] ^ data_in[16] ^
                  data_in[17] ^ data_in[20] ^ data_in[21] ^ data_in[24] ^ data_in[25] ^
                  data_in[27] ^ data_in[28] ^ data_in[31] ^ data_in[32] ^ data_in[35] ^
                  data_in[36] ^ data_in[39] ^ data_in[40];

  paritycalc[2] = data_in[1] ^ data_in[2] ^ data_in[3] ^ data_in[7] ^ data_in[8] ^
                  data_in[9] ^ data_in[10] ^ data_in[14] ^ data_in[15] ^ data_in[16] ^
                  data_in[17] ^ data_in[22] ^ data_in[23] ^ data_in[24] ^ data_in[25] ^
                  data_in[29] ^ data_in[30] ^ data_in[31] ^ data_in[32] ^ data_in[37] ^
                  data_in[38] ^ data_in[39] ^ data_in[40];

  paritycalc[3] = data_in[4] ^ data_in[5] ^ data_in[6] ^ data_in[7] ^ data_in[8] ^
                  data_in[9] ^ data_in[10] ^ data_in[18] ^ data_in[19] ^ data_in[20] ^
                  data_in[21] ^ data_in[22] ^ data_in[23] ^ data_in[24] ^ data_in[25] ^
                  data_in[33] ^ data_in[34] ^ data_in[35] ^ data_in[36] ^ data_in[37] ^
                  data_in[38] ^ data_in[39] ^ data_in[40];

  paritycalc[4] = data_in[11] ^ data_in[12] ^ data_in[13] ^ data_in[14] ^ data_in[15] ^
                  data_in[16] ^ data_in[17] ^ data_in[18] ^ data_in[19] ^ data_in[20] ^
                  data_in[21] ^ data_in[22] ^ data_in[23] ^ data_in[24] ^ data_in[25] ^
                  data_in[41] ^ data_in[42];

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

int parity(uint64_t v)
{
  // return the even parity of v
  std::bitset<64> bs(v);
  return (bs.count() + 1) % 2 == 0;
}

std::bitset<43> data43(uint64_t header)
{
  // extract the 43 data bits from a 50-bits header
  uint64_t x = (header >> 7) & 0x7FFFFFFFFFF;
  return std::bitset<43>(x);
}

int originalPos(int pos)
{
  // original pos in a word of 50 bits with hamming redundant
  // bits added, but in a word of 43 bits where they were not added

  // position of redundant bits are "not allowed"
  if (pos == 0 || pos == 1 || pos == 3 || pos == 7 || pos == 15 || pos == 31) {
    return -1;
  }
  if (pos > 31) {
    return pos - 6;
  }
  if (pos > 15) {
    return pos - 5;
  }
  if (pos > 7) {
    return pos - 4;
  }
  if (pos > 3) {
    return pos - 3;
  }
  if (pos == 2) {
    return 0;
  }
  return -1;
}

int partialParity(const std::bitset<43>& bs, int pos)
{
  // compute the parity of all the bits at position x
  // (where x & (2^pos)) are set
  int n{0};
  uint64_t one{1};
  std::vector<int> ipos;
  std::vector<int> tpos;
  for (uint64_t i = 0; i < 49; i++) {
    if (i & (one << pos)) {
      int opos = originalPos(i);
      ipos.push_back(i);
      tpos.push_back(opos);
      if (opos < 0) {
        continue;
      }
      if (bs.test(opos)) {
        ++n;
      }
    }
  }
  std::cout << "bs=" << bs << " pos=" << pos << "\n";
  std::cout << "tested: " << ipos.size() << " " << tpos.size() << "\n";
  for (auto v : ipos) {
    std::cout << fmt::format("{:3d}", v) << " ";
  }
  std::cout << "\n";
  for (auto v : tpos) {
    std::cout << fmt::format("{:3d}", v) << " ";
  }
  std::cout << "\n";
  return (n + 1) % 2 == 0;
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

BOOST_AUTO_TEST_CASE(Parity)
{
  uint64_t one{1};
  std::vector<uint64_t> numbers = {1, 2, 3, 42, 1245, one << 49 | one << 32};

  for (auto n : numbers) {
    std::cout << fmt::format("P{}={} {}\n",
                             n, parity(n), CheckDataParity(n));
    BOOST_CHECK_EQUAL(parity(n), CheckDataParity(n) == 1);
  }
}

BOOST_AUTO_TEST_CASE(PartialParity)
{
  BOOST_CHECK_EQUAL(partialParity(15, 0), 0);
  BOOST_CHECK_EQUAL(partialParity(15, 1), 0);
  BOOST_CHECK_EQUAL(partialParity(15, 3), 0);
}

// return the header parity (not using the parity bit itself)
int headerParity(uint64_t value)
{
  std::bitset<50> header(value);
  header.reset(6);
  return parity(header.to_ulong());
}

int hammingCode(uint64_t value)
{
  // value is assumed to be 50 bits, where the 43 data bits
  // are 7-49
  SampaHeader h(value);
  int p = headerParity(value);
  std::cout << h << " P=" << p << "\n";
  auto bs = data43(value);
  std::string s = bs.to_string();
  std::reverse(s.begin(), s.end());
  std::cout << "bs=" << s << "\n";
  for (int i = 0; i < 6; i++) {
    std::cout << "i=" << i << " -> " << partialParity(bs, i) << "\n";
  }
  return p;
}

BOOST_AUTO_TEST_CASE(TestHamming)
{
  BOOST_CHECK_EQUAL(hammingCode(0x3722e80103208), 0x8); // 000100 P0
  // BOOST_CHECK_EQUAL(hammingCode(0x1722e8090322f), 0x2F); // 111101 P0
  // BOOST_CHECK_EQUAL(hammingCode(0x1722e9f00327d), 0x2D); // 101101 P1

  for (int i = 0; i < 50; i++) {
    std::cout << fmt::format("{:3d}", i);
  }
  std::cout << "\n";
  for (int i = 0; i < 50; i++) {
    std::cout << fmt::format("{:3d}", originalPos(i));
  }
  std::cout << "\n";
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
