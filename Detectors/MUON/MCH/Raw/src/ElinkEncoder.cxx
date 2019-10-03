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
#include "MCHRaw/SampaHeader.h"
#include <iostream>
#include <fmt/printf.h>
#include <fmt/format.h>
#include "MCHRaw/BitSet.h"
#include "NofBits.h"
#include "CompactBitSetString.h"

namespace o2::mch::raw
{
const BitSet sync(sampaSync().uint64(), 50);

void computeHamming(SampaHeader& sampaHeader)
{
  // FIXME: compute hamming and parities
}

ElinkEncoder::ElinkEncoder(uint8_t id, uint8_t dsid, int phase) : mId(id), mDsId(dsid), mSampaHeader{}, mBitSet{}, mNofSync{0}, mSyncIndex{0}, mNofBitSeen{0}
{
  if (id > 39) {
    throw std::invalid_argument(fmt::sprintf("id = %d should be between 0 and 39", id));
  }

  // the phase is used to "simulate" a possible different timing alignment between elinks. Just using here as random bits (does not really matter).

  if (phase < 0) {
    phase = static_cast<int>(rand() % 20);
  }
  for (int i = 0; i < phase; i++) {
    mBitSet.append(static_cast<bool>(rand() % 2));
    mNofBitSeen++;
  }

  mSampaHeader.chipAddress(mDsId);
}

void ElinkEncoder::addHeader(uint8_t chId, const std::vector<uint16_t>& samples)
{
  int n10 = 2; // nofsamples + timestamp
  // check all samples are 10 bits
  for (auto s : samples) {
    assertNofBits("sample", s, 10);
    n10++;
  }
  setHeader(chId, n10);
  // append header to bitset
  mBitSet.append(mSampaHeader.uint64(), 50);
  mNofBitSeen += 50;
}

void ElinkEncoder::addHeader(uint8_t chId, uint32_t chargeSum)
{
  int n10 = 4; // nofsamples + timestamp + 20 bits of chargeSum
  assertNofBits("chargeSum", chargeSum, 20);
  setHeader(chId, n10);
  // append header to bitset
  mBitSet.append(mSampaHeader.uint64(), 50);
  mNofBitSeen += 50;
}

void ElinkEncoder::addChannelSamples(uint8_t chId,
                                     uint16_t timestamp,
                                     const std::vector<uint16_t>& samples)
{
  assertSync();

  addHeader(chId, samples);

  assertNofBits("timestamp", timestamp, 10);

  // append samples to bitset
  mBitSet.append(static_cast<uint16_t>(samples.size()), 10);
  mBitSet.append(timestamp, 10);
  mNofBitSeen += 20;

  for (auto s : samples) {
    mBitSet.append(s, 10);
    mNofBitSeen += 10;
  }
}

void ElinkEncoder::addChannelChargeSum(uint8_t chId,
                                       uint16_t timestamp,
                                       uint32_t chargeSum,
                                       uint16_t nsamples)
{
  assertSync();

  addHeader(chId, chargeSum);

  // append samples to bitset
  assertNofBits("nsamples", nsamples, 10);
  assertNofBits("timestamp", timestamp, 10);

  mBitSet.append(nsamples, 10);
  mBitSet.append(timestamp, 10);
  mBitSet.append(chargeSum, 20);
  mNofBitSeen += 40;
}

void ElinkEncoder::addTestBit(bool value)
{
  mBitSet.append(value);
  mNofBitSeen++;
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
      mBitSet.append(sync.get(i));
      mNofBitSeen++;
    }
    mSyncIndex = 0;
    if (firstSync) {
      mNofSync++;
    }
  }
}

void ElinkEncoder::bunchCrossingCounter(uint32_t bx)
{
  assertNofBits("bx", bx, 20);
  mSampaHeader.bunchCrossingCounter(bx);
}

void ElinkEncoder::fillWithSync(int upto)
{
  auto d = upto - len();
  mSyncIndex = circularAppend(mBitSet, sync, mSyncIndex, d);
  mNofSync += d / 50;
  mNofBitSeen += d;
}

uint64_t ElinkEncoder::range(int a, int b) const
{
  return mBitSet.subset(a, b).uint64(0, b - a + 1);
}

std::ostream& operator<<(std::ostream& os, const ElinkEncoder& enc)
{
  os << fmt::sprintf("ELINK ID %2d DSID %2d nsync %3llu len %6llu syncindex %2d nbitseen %10d | %s",
                     enc.mId, enc.mDsId, enc.mNofSync, enc.mBitSet.len(),
                     enc.mSyncIndex, enc.mNofBitSeen,
                     compactString(enc.mBitSet));
  return os;
}

void ElinkEncoder::clear()
{
  mBitSet.clear();
}

bool ElinkEncoder::get(int i) const
{
  if (i >= len()) {
    throw std::invalid_argument(fmt::format("trying to access bit {} which is past the end of our bitset of len {}", i, len()));
  }
  return mBitSet.get(i);
}

uint8_t ElinkEncoder::id() const
{
  return mDsId;
}

int ElinkEncoder::len() const
{
  return mBitSet.len();
}

void ElinkEncoder::setHeader(uint8_t chId, uint16_t n10)
{
  assertNofBits("chId", chId, 5);
  assertNofBits("nof10BitWords", n10, 10);
  mSampaHeader.packetType(SampaPacketType::Data);
  mSampaHeader.nof10BitWords(n10);
  mSampaHeader.channelAddress(chId);
  computeHamming(mSampaHeader);
}

} // namespace o2::mch::raw
