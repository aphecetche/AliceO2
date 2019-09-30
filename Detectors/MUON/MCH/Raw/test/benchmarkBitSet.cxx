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

#include "MCHRaw/BitSet.h"

using namespace o2::mch::raw;

static void BM_NofBits8(benchmark::State& state)
{
  int nok{0};
  for (auto _ : state) {
    if (nofBits(static_cast<uint16_t>(255)) > 8) {
      nok++;
    }
  }
}

static void BM_NofBits32(benchmark::State& state)
{
  int nok{0};
  for (auto _ : state) {
    if (nofBits(static_cast<uint32_t>(255)) > 8) {
      nok++;
    }
  }
}

static void BM_NofBits64(benchmark::State& state)
{
  int nok{0};
  for (auto _ : state) {
    if (nofBits(static_cast<uint64_t>(255)) > 8) {
      nok++;
    }
  }
}

BENCHMARK(BM_NofBits8);
BENCHMARK(BM_NofBits32);
BENCHMARK(BM_NofBits64);

BENCHMARK_MAIN();
