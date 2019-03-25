// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "benchmark/benchmark.h"
#include "MCHSimulation/Digit.h"
#include <algorithm>
#include <numeric>
#include <iostream>

using o2::mch::Digit;

std::vector<Digit> mergeDigitsA(const std::vector<Digit>& inputDigits)
{
  int iter = 0;
  int count = 0;
  int index = 0;
  std::map<int, int> padidmap;
  std::set<int> forRemoval;
  std::vector<Digit> digits{ inputDigits };

  for (auto& digit : digits) {
    int padid = digit.getPadID();
    count = padidmap.count(padid);
    if (count) {
      std::pair<std::map<int, int>::iterator, std::map<int, int>::iterator> ret;

      ret = padidmap.equal_range(padid);
      index = ret.first->second;
      (digits.at(index)).setADC(digits.at(index).getADC() + digit.getADC());
      forRemoval.emplace(iter);
    } else {
      padidmap.emplace(padid, iter);
    }
    ++iter;
  }

  int rmcounts = 0;
  for (auto& rmindex : forRemoval) {
    digits.erase(digits.begin() + rmindex - rmcounts);
    ++rmcounts;
  }
  return digits;
}

std::vector<Digit> mergeDigitsB(const std::vector<Digit>& inputDigits)
{
  std::vector<int> indices(inputDigits.size());
  std::iota(begin(indices), end(indices), 0);

  std::sort(indices.begin(), indices.end(), [&inputDigits](int a, int b) {
    return inputDigits[a].getPadID() < inputDigits[b].getPadID();
  });

  std::vector<Digit> digits(inputDigits.size());
  for (auto i : indices) {
    digits[i] = inputDigits[indices[i]];
  }
  return digits;
}

void dumpDigits(const std::vector<Digit>& digits)
{
  for (auto& d : digits) {
    std::cout << "[" << d.getPadID() << "," << d.getADC() << " ] ";
  }
  std::cout << "\n";
}
std::vector<Digit> createDigits(int N)
{
  std::vector<Digit> digits;
  float dummyadc{ 42.42 };
  for (auto i = 0; i < N; i++) {
    digits.emplace_back(i, dummyadc);
  }

  return digits;
}

static void benchDigitMergingA(benchmark::State& state)
{
  auto digits = createDigits(100);

  for (auto _ : state) {
    mergeDigitsA(digits);
  }
}

static void benchDigitMergingB(benchmark::State& state)
{
  auto digits = createDigits(10);

  for (auto _ : state) {
    mergeDigitsB(digits);
  }
}

struct A {
  A()
  {
    std::cout << "AAAAAAAAAAAA\n";
    std::vector<Digit> digits;
    digits.emplace_back(2, 5);
    digits.emplace_back(3, 6);
    digits.emplace_back(1, 2);
    digits.emplace_back(0, 0);
    digits.emplace_back(0, 1);
    digits.emplace_back(1, 3);
    digits.emplace_back(3, 7);
    digits.emplace_back(1, 4);
    dumpDigits(digits);
    dumpDigits(mergeDigitsB(digits));
  }
} a;

// BENCHMARK(benchDigitMergingA);
BENCHMARK(benchDigitMergingB);

BENCHMARK_MAIN();
