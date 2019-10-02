// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHRaw/GBTEncoder.h"
#include <fmt/printf.h>
#include <stdexcept>
#include "MakeArray.h"

using namespace o2::mch::raw;

// FIXME: instead of i % 16 for dsid , get a "real" mapping in there
GBTEncoder::GBTEncoder(int linkId) : mId(linkId), mElinks{::makeArray<40>([](size_t i) { return ElinkEncoder(i, i % 16, i); })}, mGBTWords{}
{
  if (linkId < 0 || linkId > 23) {
    throw std::invalid_argument(fmt::sprintf("linkId %d should be between 0 and 23", linkId));
  }
}

void GBTEncoder::addChannelChargeSum(uint32_t bx, uint8_t elinkId, uint8_t chId, uint16_t timestamp, uint32_t chargeSum)
{
  if (elinkId < 0 || elinkId > 39) {
    throw std::invalid_argument(fmt::sprintf("elinkId %d should be between 0 and 39", elinkId));
  }
  mElinks[elinkId].bunchCrossingCounter(bx);
  mElinks[elinkId].addChannelChargeSum(chId, timestamp, chargeSum);
}

void GBTEncoder::elink2gbt()
{
  // convert elinks content to actual GBT words
  if (!areElinksAligned()) {
    throw std::logic_error("should not call elink2gbt before all elinks are synched !");
  }

  int n = mElinks[0].len();

  for (int i = 0; i < n - 1; i += 2) {
    uint128_t w{0};
    for (int j = 0; j < 80; j += 2) {
      for (int k = 0; k <= 1; k++) {
        bool v = mElinks[j / 2].get(i + k);
        if (v) {
          bit_set(w, j + k);
        } else {
          bit_unset(w, j + k);
        }
      }
    }
    mGBTWords.push_back(w);
  }
}

int GBTEncoder::len() const
{
  auto e = std::max_element(begin(mElinks), end(mElinks),
                            [](const ElinkEncoder& a, const ElinkEncoder& b) {
                              return a.len() < b.len();
                            });
  return e->len();
}

bool GBTEncoder::areElinksAligned() const
{
  auto len = mElinks[0].len();
  for (auto i = 1; i < mElinks.size(); i++) {
    if (mElinks[i].len() != len) {
      return false;
    }
  }
  return true;
}

void GBTEncoder::align(int upto)
{
  // align all elink sizes by adding sync bits
  for (auto i = 0; i < mElinks.size(); i++) {
    mElinks[i].fillWithSync(upto);
  }
}

void GBTEncoder::finalize(int alignToSize)
{
  if (areElinksAligned()) {
    return;
  }

  // compute align size if not given
  if (alignToSize <= 0) {
    alignToSize = len();
  }

  std::cout << "GBTEncoder::finalize before align\n";
  printStatus();

  // align sizes of all elinks by adding sync bits
  align(alignToSize);

  std::cout << "GBTEncoder::finalize after align\n";
  printStatus();

  // convert elinks to GBT words
  elink2gbt();

  std::cout << "GBTEncoder::finalize after elink2gbt\n";
  printStatus();

  // reset all the links
  clear();
}

void GBTEncoder::clear()
{
  // clear the elinks
  for (auto i = 0; i < mElinks.size(); i++) {
    mElinks[i].clear();
  }
}

size_t GBTEncoder::size() const
{
  return mGBTWords.size();
}

uint128_t GBTEncoder::getWord(int i) const
{
  return mGBTWords[i];
}

void GBTEncoder::printStatus() const
{
  std::cout << fmt::format("GBTEncoder({}) elinks are in sync : {} # GBTwords : {} len {}\n", mId, areElinksAligned(), mGBTWords.size(), len());
  // for (auto& e : mElinks) { // FIXME:
  for (int i = 0; i < 12; i++) {
    const auto& e = mElinks[i];
    std::cout << e << "\n";
  }
}
