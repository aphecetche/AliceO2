// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include <benchmark/benchmark.h>
#include <bitset>
#include <array>

constexpr uint64_t one{1};

bool parity(uint64_t v)
{
  // return the even parity of v
  std::bitset<64> bs(v);
  return (bs.count() + 1) % 2 == 0;
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

constexpr std::array<uint64_t, 64> bit = {
  one << 0,
  one << 1,
  one << 2,
  one << 3,
  one << 4,
  one << 5,
  one << 6,
  one << 7,
  one << 8,
  one << 9,
  one << 10,
  one << 11,
  one << 12,
  one << 13,
  one << 14,
  one << 15,
  one << 16,
  one << 17,
  one << 18,
  one << 19,
  one << 20,
  one << 21,
  one << 22,
  one << 23,
  one << 24,
  one << 25,
  one << 26,
  one << 27,
  one << 28,
  one << 29,
  one << 30,
  one << 31,
  one << 32,
  one << 33,
  one << 34,
  one << 35,
  one << 36,
  one << 37,
  one << 38,
  one << 39,
  one << 40,
  one << 41,
  one << 42,
  one << 43,
  one << 44,
  one << 45,
  one << 46,
  one << 47,
  one << 48,
  one << 49,
  one << 50,
  one << 51,
  one << 52,
  one << 53,
  one << 54,
  one << 55,
  one << 56,
  one << 57,
  one << 58,
  one << 59,
  one << 60,
  one << 61,
  one << 62,
  one << 63};

int countBitsArray(uint64_t value)
{
  int n{0};

  for (int i = 0; i < 64; i++) {
    if (value & bit[i]) {
      ++n;
    }
  }
  return n;
}

int countBitsBasic(uint64_t value)
{
  int n{0};

  for (int i = 0; i < 64; i++) {
    if (value & (one << i)) {
      ++n;
    }
  }
  return n;
}
static void BM_Parity(benchmark::State& state)
{
  for (auto _ : state) {
    parity(1);
  }
}

static void BM_ParityBasic(benchmark::State& state)
{
  for (auto _ : state) {
    countBitsBasic(1);
  }
}
static void BM_ParityArray(benchmark::State& state)
{
  for (auto _ : state) {
    countBitsArray(1);
  }
}

static void BM_ParityBitset(benchmark::State& state)
{
  for (auto _ : state) {
    std::bitset<64> bs(1);
    bs.count();
  }
}

static void BM_ParityCheckDataParity(benchmark::State& state)
{
  for (auto _ : state) {
    CheckDataParity(1);
  }
}
// Register the function as a benchmark
BENCHMARK(BM_ParityArray);
BENCHMARK(BM_ParityBasic);
BENCHMARK(BM_ParityBitset);
BENCHMARK(BM_Parity);
BENCHMARK(BM_ParityCheckDataParity);

// Run the benchmark
BENCHMARK_MAIN();
