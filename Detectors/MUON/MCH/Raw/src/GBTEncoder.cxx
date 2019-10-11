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
#include "Assertions.h"

using namespace o2::mch::raw;

int phase(int i);

bool GBTEncoder::forceNoPhase{false};

// FIXME: instead of i % 16 for dsid , get a "real" mapping in there
GBTEncoder::GBTEncoder(int cruId, int linkId) : mCruId(cruId), mGbtId(linkId), mElinks{::makeArray<40>([](size_t i) { return ElinkEncoder(i, i % 16, phase(i)); })}, mGbtWords{}
{
  assertIsInRange("linkId", linkId, 0, 23);
}

void GBTEncoder::addChannelChargeSum(uint8_t elinkId, uint16_t timestamp, uint8_t chId, uint32_t chargeSum)
{
  assertIsInRange("elinkId", elinkId, 0, 39);
  mElinks[elinkId].addChannelData(chId, {SampaCluster{timestamp, chargeSum}});
}

void GBTEncoder::align(int upto)
{
  // align all elink sizes by adding sync bits
  for (auto i = 0; i < mElinks.size(); i++) {
    mElinks[i].fillWithSync(upto);
  }
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

void GBTEncoder::clear()
{
  // clear the elinks
  for (auto i = 0; i < mElinks.size(); i++) {
    mElinks[i].clear();
  }
}

void GBTEncoder::elink2gbt()
{
  // convert elinks content to actual GBT words
  if (!areElinksAligned()) {
    throw std::logic_error("should not call elink2gbt before all elinks are synched !");
  }

  auto prev = mGbtWords.size();

  int n = mElinks[0].len();

  for (int i = 0; i < n - 1; i += 2) {
    uint128_t w{0};
    for (int j = 0; j < 80; j += 2) {
      for (int k = 0; k <= 1; k++) {
        bool v = mElinks[j / 2].get(i + 1 - k);
        if (v) {
          bit_set(w, j + k);
        } else {
          bit_unset(w, j + k);
        }
      }
    }
    mGbtWords.push_back(w);
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

  // align sizes of all elinks by adding sync bits
  align(alignToSize);

  // convert elinks to GBT words
  elink2gbt();

  // reset all the links
  clear();
}

uint128_t GBTEncoder::getWord(int i) const
{
  return mGbtWords[i];
}

/// len returns the maximum number of bits currently stored
/// in our Elinks (i.e. the numbe of bits of the widest of
/// our 40 elinks).
int GBTEncoder::len() const
{
  auto e = std::max_element(begin(mElinks), end(mElinks),
                            [](const ElinkEncoder& a, const ElinkEncoder& b) {
                              return a.len() < b.len();
                            });
  return e->len();
}

void GBTEncoder::printStatus(int maxelink) const
{
  std::cout << fmt::format("GBTEncoder({}) elinks are in sync : {} # GBTwords : {} len {}\n", mGbtId, areElinksAligned(), mGbtWords.size(), len());
  auto n = mElinks.size();
  if (maxelink > 0 && maxelink <= n) {
    n = maxelink;
  }
  for (int i = 0; i < n; i++) {
    const auto& e = mElinks[i];
    std::cout << e << "\n";
  }
}

size_t GBTEncoder::moveToBuffer(std::vector<uint32_t>& buffer)
{
  finalize();
  constexpr uint128_t m = 0xFFFFFFFFuLL;
  uint128_t w0 = m;
  uint128_t w1 = m << 32;
  uint128_t w2 = m << 64;
  uint128_t w3 = m << 96;
  size_t n{0};
  for (auto& g : mGbtWords) {
    buffer.emplace_back(static_cast<uint32_t>(g & m));
    buffer.emplace_back(static_cast<uint32_t>((g & w1) >> 32));
    buffer.emplace_back(static_cast<uint32_t>((g & w2) >> 64));
    buffer.emplace_back(static_cast<uint32_t>((g & w3) >> 96));
    n += 4;
  }
  mGbtWords.clear();
  return n;
}

void GBTEncoder::resetLocalBunchCrossing()
{
  for (auto i = 0; i < mElinks.size(); i++) {
    mElinks[i].resetLocalBunchCrossing();
  }
}

size_t GBTEncoder::size() const
{
  return mGbtWords.size();
}

int phase(int i)
{
  // generate the phase for the i-th ElinkEncoder
  // the default value of -1 means it will be random and decided
  // by the ElinkEncoder ctor
  //
  // if > 0 it will set a fixed phase at the beginning of the life
  // of the ElinkEncoder
  //
  // returning zero will simply disable the phase

  if (GBTEncoder::forceNoPhase) {
    return 0;
  }
  return -1;
}
