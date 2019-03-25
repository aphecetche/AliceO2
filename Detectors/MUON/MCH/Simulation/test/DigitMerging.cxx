#include "DigitMerging.h"
#include <map>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <set>

using o2::mch::Digit;

void dumpDigits(const std::vector<Digit>& digits)
{
  int i{ 0 };
  std::cout << "dumpDigits" << std::string(40, '-') << "\n";
  for (auto& d : digits) {
    std::cout << "i=" << i << ":[" << d.getPadID() << "," << d.getADC() << " ] ";
    i++;
  }
  std::cout << "\n";
}

std::vector<Digit> mergeDigitsLA1(const std::vector<Digit>& inputDigits)
{
  std::vector<int> indices(inputDigits.size());
  std::iota(begin(indices), end(indices), 0);

  std::sort(indices.begin(), indices.end(), [&inputDigits](int a, int b) {
    return inputDigits[a].getPadID() < inputDigits[b].getPadID();
  });

  auto sortedDigits = [&inputDigits, &indices](int i) {
    return inputDigits[indices[i]];
  };

  std::vector<Digit> digits;

  int i = 0;
  while (i < indices.size()) {
    int j = i + 1;
    while (j < indices.size() && (sortedDigits(i).getPadID() == sortedDigits(j).getPadID())) {
      j++;
    }
    float adc{ 0 };
    for (int k = i; k < j; k++) {
      adc += sortedDigits(k).getADC();
    }
    digits.emplace_back(sortedDigits(i).getPadID(), adc);
    i = j;
  }
  return digits;
}

std::vector<Digit> mergeDigitsLA2(const std::vector<Digit>& inputDigits)
{
  std::vector<int> indices(inputDigits.size());
  std::iota(begin(indices), end(indices), 0);

  std::sort(indices.begin(), indices.end(), [&inputDigits](int a, int b) {
    return inputDigits[a].getPadID() < inputDigits[b].getPadID();
  });

  auto sortedDigits = [&inputDigits, &indices](int i) {
    return inputDigits[indices[i]];
  };

  std::vector<Digit> digits;
  digits.reserve(inputDigits.size());

  int i = 0;
  while (i < indices.size()) {
    int j = i + 1;
    while (j < indices.size() && (sortedDigits(i).getPadID() == sortedDigits(j).getPadID())) {
      j++;
    }
    float adc{ 0 };
    for (int k = i; k < j; k++) {
      adc += sortedDigits(k).getADC();
    }
    digits.emplace_back(sortedDigits(i).getPadID(), adc);
    i = j;
  }
  digits.resize(digits.size());
  return digits;
}

std::vector<Digit> mergeDigitsMW(const std::vector<Digit>& inputDigits)
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

std::vector<MergingFunctionType> mergingFunctions()
{
  return std::vector<MergingFunctionType>{ mergeDigitsLA1, mergeDigitsLA2, mergeDigitsMW };
}
