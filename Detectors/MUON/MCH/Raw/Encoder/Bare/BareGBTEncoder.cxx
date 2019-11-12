// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "BareGBTEncoder.h"
#include <fmt/printf.h>
#include <stdexcept>
#include "MakeArray.h"
#include "Assertions.h"
#include "BareElinkEncoder.h"
#include "MoveBuffer.h"

using namespace o2::mch::raw;

int phase(int i, bool forceNoPhase);

bool BareGBTEncoder::forceNoPhase{false};

// FIXME: instead of i % 16, get a "real" mapping in there
BareGBTEncoder::BareGBTEncoder(int cruId, int linkId, bool chargeSumMode)
  : mCruId(cruId),
    mGbtId(linkId),
    mElinks{::makeArray<40>([chargeSumMode](size_t i) { return BareElinkEncoder(i, i % 16, phase(i, BareGBTEncoder::forceNoPhase), chargeSumMode); })},
    mGbtWords{}
{
  assertIsInRange("linkId", linkId, 0, 23);
}

void BareGBTEncoder::addChannelData(uint8_t elinkId, uint8_t chId,
                                    const std::vector<SampaCluster>& data)
{
  assertIsInRange("elinkId", elinkId, 0, 39);
  mElinks[elinkId].addChannelData(chId, data);
}

void BareGBTEncoder::align(int upto)
{
  // align all elink sizes by adding sync bits
  for (auto i = 0; i < mElinks.size(); i++) {
    mElinks[i].fillWithSync(upto);
  }
}

bool BareGBTEncoder::areElinksAligned() const
{
  auto len = mElinks[0].len();
  for (auto i = 1; i < mElinks.size(); i++) {
    if (mElinks[i].len() != len) {
      return false;
    }
  }
  return true;
}

void BareGBTEncoder::clear()
{
  // clear the elinks
  for (auto i = 0; i < mElinks.size(); i++) {
    mElinks[i].clear();
  }
}

uint64_t BareGBTEncoder::aggregate(int jstart, int jend, int i) const
{
  uint64_t w{0};
  for (int j = jstart; j < jend; j += 2) {
    for (int k = 0; k <= 1; k++) {
      bool v = mElinks[j / 2].get(i + 1 - k);
      uint64_t mask = static_cast<uint64_t>(1) << (j + k);
      if (v) {
        w |= mask;
      } else {
        w &= ~mask;
      }
    }
  }
  return w;
}

void BareGBTEncoder::elink2gbt()
{
  // convert elinks content to actual GBT words
  if (!areElinksAligned()) {
    throw std::logic_error("should not call elink2gbt before all elinks are synched !");
  }

  auto prev = mGbtWords.size();

  int n = mElinks[0].len();

  for (int i = 0; i < n - 1; i += 2) {
    uint64_t w0 = aggregate(0, 64, i);
    uint64_t w1 = aggregate(64, 80, i);
    mGbtWords.push_back(w0);
    mGbtWords.push_back(w1);
  }
}

void BareGBTEncoder::finalize(int alignToSize)
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

/// len returns the maximum number of bits currently stored
/// in our Elinks (i.e. the numbe of bits of the widest of
/// our 40 elinks).
int BareGBTEncoder::len() const
{
  auto e = std::max_element(begin(mElinks), end(mElinks),
                            [](const BareElinkEncoder& a, const BareElinkEncoder& b) {
                              return a.len() < b.len();
                            });
  return e->len();
}

void BareGBTEncoder::printStatus(int maxelink) const
{
  std::cout << fmt::format("BareGBTEncoder({}) elinks are in sync : {} # GBTwords : {} len {}\n", mGbtId, areElinksAligned(), mGbtWords.size(), len());
  auto n = mElinks.size();
  if (maxelink > 0 && maxelink <= n) {
    n = maxelink;
  }
  for (int i = 0; i < n; i++) {
    const auto& e = mElinks[i];
    std::cout << e << "\n";
  }
}

size_t BareGBTEncoder::moveToBuffer(std::vector<uint8_t>& buffer)
{
  finalize();
  return moveBuffer(mGbtWords, buffer);
}

void BareGBTEncoder::resetLocalBunchCrossing()
{
  for (auto i = 0; i < mElinks.size(); i++) {
    mElinks[i].resetLocalBunchCrossing();
  }
}

size_t BareGBTEncoder::size() const
{
  return mGbtWords.size();
}

int phase(int i, bool forceNoPhase)
{
  // generate the phase for the i-th ElinkEncoder
  // the default value of -1 means it will be random and decided
  // by the ElinkEncoder ctor
  //
  // if > 0 it will set a fixed phase at the beginning of the life
  // of the ElinkEncoder
  //
  // returning zero will simply disable the phase

  if (forceNoPhase) {
    return 0;
  }
  return -1;
}
