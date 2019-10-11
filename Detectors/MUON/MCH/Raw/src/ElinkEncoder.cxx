// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHRaw/ElinkEncoder.h"
#include "Assertions.h"
#include "CompactBitSetString.h"
#include "MCHRaw/BitSet.h"
#include "MCHRaw/SampaHeader.h"
#include "NofBits.h"
#include <fmt/format.h>
#include <fmt/printf.h>
#include <iostream>

namespace o2::mch::raw
{
const BitSet sync(sampaSync().uint64(), 50);

ElinkEncoder::ElinkEncoder(uint8_t id,
                           uint8_t chip,
                           int phase,
                           bool chargeSumMode)
  : mId(id),
    mChipAddress(chip),
    mSampaHeader{},
    mBitSet{},
    mNofSync{0},
    mSyncIndex{0},
    mNofBitSeen{0},
    mLocalBunchCrossing{0},
    mPhase{phase},
    mChargeSumMode{chargeSumMode}
{
  assertIsInRange("id", id, 0, 39);

  // the phase is used to "simulate" a possible different timing alignment between elinks.

  if (phase < 0) {
    mPhase = static_cast<int>(rand() % 20);
  }

  for (int i = 0; i < mPhase; i++) {
    // filling the phase with random bits
    append(static_cast<bool>(rand() % 2));
  }

  mSampaHeader.chipAddress(mChipAddress);
}

void ElinkEncoder::assertNotMixingClusters(const std::vector<SampaCluster>& data) const
{
  // ensure all clusters are either in sample mode or in
  // chargesum mode, no mixing allowed
  assert(data.size() > 0);
  for (auto i = 0; i < data.size(); i++) {
    if (data[i].isClusterSum() != mChargeSumMode) {
      throw std::invalid_argument(fmt::format("all cluster of this encoder should be of the same type ({}) but {}-th does not match ", (mChargeSumMode ? "clusterSum" : "samples"), i));
    }
  }
}

void ElinkEncoder::addChannelData(uint8_t chId, const std::vector<SampaCluster>& data)
{
  if (data.empty()) {
    throw std::invalid_argument("cannot add empty data");
  }
  assertSync();
  assertNotMixingClusters(data);

  uint16_t n10{0};
  for (const auto& s : data) {
    n10 += s.nof10BitWords();
  }

  setHeader(chId, n10);
  append50(mSampaHeader.uint64());
  for (const auto& s : data) {
    append(s);
  }
}

void ElinkEncoder::append(const SampaCluster& sc)
{
  append10(sc.nofSamples());
  append10(sc.timestamp);
  if (mChargeSumMode) {
    append20(sc.chargeSum);
  } else {
    for (auto& s : sc.samples) {
      append10(s);
    }
  }
}

void ElinkEncoder::append(bool value)
{
  mBitSet.append(value);
  mNofBitSeen++;
}

void ElinkEncoder::append10(uint16_t value)
{
  mBitSet.append(value, 10);
  mNofBitSeen += 10;
}

void ElinkEncoder::append20(uint32_t value)
{
  mBitSet.append(value, 20);
  mNofBitSeen += 20;
}

void ElinkEncoder::append50(uint64_t value)
{
  mBitSet.append(value, 50);
  mNofBitSeen += 50;
}

void ElinkEncoder::assertSync()
{
  bool firstSync = (mNofSync == 0);

  // if mSyncIndex is not zero it means
  // we have a pending sync to finish to transmit
  // (said otherwise we're not aligned to an expected 50bits mark)
  bool pendingSync = (mSyncIndex != 0);

  if (firstSync || pendingSync) {
    for (int i = mSyncIndex; i < 50; i++) {
      append(sync.get(i));
    }
    mSyncIndex = 0;
    if (firstSync) {
      mNofSync++;
    }
  }
}

void ElinkEncoder::clear()
{
  mBitSet.clear();
}

void ElinkEncoder::fillWithSync(int upto)
{
  auto d = upto - len();
  mSyncIndex = circularAppend(mBitSet, sync, mSyncIndex, d);
  mNofSync += d / 50;
  mNofBitSeen += d;
}

bool ElinkEncoder::get(int i) const
{
  assertIsInRange("i", i, 0, len() - 1);
  return mBitSet.get(i);
}

uint8_t ElinkEncoder::id() const
{
  return mChipAddress;
}

int ElinkEncoder::len() const
{
  return mBitSet.len();
}

uint64_t ElinkEncoder::range(int a, int b) const
{
  return mBitSet.subset(a, b).uint64(0, b - a + 1);
}

void ElinkEncoder::resetLocalBunchCrossing()
{
  mLocalBunchCrossing = mPhase;
}

void ElinkEncoder::setHeader(uint8_t chId, uint16_t n10)
{
  assertNofBits("chId", chId, 5);
  assertNofBits("nof10BitWords", n10, 10);
  mSampaHeader.bunchCrossingCounter(mLocalBunchCrossing);
  mSampaHeader.packetType(SampaPacketType::Data);
  mSampaHeader.nof10BitWords(n10);
  mSampaHeader.channelAddress(chId);
}

std::ostream& operator<<(std::ostream& os, const ElinkEncoder& enc)
{
  os << fmt::sprintf("ELINK ID %2d chip %2d nsync %3llu len %6llu syncindex %2d nbitseen %10d %10s | %s",
                     enc.mId, enc.mChipAddress, enc.mNofSync, enc.mBitSet.len(),
                     enc.mSyncIndex, enc.mNofBitSeen,
                     (enc.mChargeSumMode ? "CLUSUM" : "SAMPLE"),
                     compactString(enc.mBitSet));
  return os;
}

} // namespace o2::mch::raw
