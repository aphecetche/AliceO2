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
#include "MCHRawCommon/SampaHeader.h"

using namespace o2::mch::raw;

static void BM_ComputeHamming(benchmark::State& state)
{
  for (auto _ : state) {
    benchmark::DoNotOptimize(computeHammingCode(0x3722e80103208));
  }
}

static void BM_ComputeHamming2(benchmark::State& state)
{
  for (auto _ : state) {
    benchmark::DoNotOptimize(computeHammingCode2(0x3722e80103208));
  }
}

static void BM_ComputeHamming3(benchmark::State& state)
{
  for (auto _ : state) {
    benchmark::DoNotOptimize(computeHammingCode3(0x3722e80103208));
  }
}

// Register the function as a benchmark
BENCHMARK(BM_ComputeHamming);
BENCHMARK(BM_ComputeHamming2);
BENCHMARK(BM_ComputeHamming3);

// Run the benchmark
BENCHMARK_MAIN();

// BOOST_AUTO_TEST_CASE(ComputeHammingCode)
// {
//   BOOST_CHECK_EQUAL(computeHammingCode(0x3722e80103208), 0x8);  // 000100 P0
//   BOOST_CHECK_EQUAL(computeHammingCode(0x1722e9f00327d), 0x3D); // 101101 P1
//   BOOST_CHECK_EQUAL(computeHammingCode(0x1722e8090322f), 0x2F); // 111101 P0
// }
