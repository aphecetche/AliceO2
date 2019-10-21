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

void GBTDecoder::append(uint32_t w, int offset, int n)
{
  uint32_t m{1};
  int elinkIndex = offset / 2;

  for (int i = 0; i < n; i += 2) {
    mElinks[elinkIndex].append(w & (m * 2), w & m);
    m *= 4;
    ++elinkIndex;
  }
}

void GBTDecoder::append(uint32_t w0, uint32_t w1, uint32_t w2, uint32_t /* w3 */)
{
  ++mNofGbtWordsSeens;
  append(w0, 0, 32);
  append(w1, 32, 32);
  append(w2, 64, 16);
  // w3 is not used as only the first 80 bits are of interest to us
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
