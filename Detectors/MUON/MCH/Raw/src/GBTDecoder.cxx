// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHRaw/GBTDecoder.h"
#include <fmt/printf.h>
#include <fmt/format.h>
#include "MakeArray.h"
#include "Assertions.h"
#include <iostream>

using namespace o2::mch::raw;
using namespace boost::multiprecision;

GBTDecoder::GBTDecoder(int cruId,
                       int gbtId,
                       SampaChannelHandler sampaChannelHandler,
                       bool chargeSumMode)
  : mCruId(cruId),
    mGbtId(gbtId),
    mElinks{::makeArray<40>([=](size_t i) { return ElinkDecoder(cruId, i, sampaChannelHandler, chargeSumMode); })},
    mNofGbtWordsSeens{0}
{
  assertIsInRange("gbtId", gbtId, 0, 23);
}

void GBTDecoder::append(gsl::span<uint8_t> bytes)
{
  if (bytes.size() % 16 != 0) {
    throw std::invalid_argument("can only bytes by group of 16 (i.e. 128 bits)");
  }
  for (int j = 0; j < bytes.size(); j += 16) {
    if (j % 16 > 10) {
      // only consider 80 bits of each 128 bits group
      continue;
    }
    ++mNofGbtWordsSeens;
    int elinkIndex = 0;
    for (auto b : bytes.subspan(j, 10)) {
      mElinks[elinkIndex++].append(b & 2, b & 1);
      mElinks[elinkIndex++].append(b & 8, b & 4);
      mElinks[elinkIndex++].append(b & 32, b & 16);
      mElinks[elinkIndex++].append(b & 128, b & 64);
    }
  }
}

void GBTDecoder::printStatus(int maxelink) const
{
  std::cout << fmt::format("GBTDecoder(CRU{}-GBT{}) # GBT words seen {}\n", mCruId, mGbtId, mNofGbtWordsSeens);
  auto n = mElinks.size();
  if (maxelink > 0) {
    n = maxelink;
  }
  for (int i = 0; i < n; i++) {
    const auto& e = mElinks[i];
    std::cout << e << "\n";
  }
}

void GBTDecoder::reset()
{
  for (auto& e : mElinks) {
    e.reset();
  }
}
