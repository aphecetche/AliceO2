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

void computeHamming(SampaHeader& sampaHeader)
{
  // FIXME: compute hamming and parities
}

ElinkEncoder::ElinkEncoder(uint8_t id, uint8_t chip, int phase) : mId(id), mChipAddress(chip), mSampaHeader{}, mBitSet{}, mNofSync{0}, mSyncIndex{0}, mNofBitSeen{0}, mLocalBunchCrossing{0}, mPhase{phase}
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

void ElinkEncoder::addChannelSamples(uint8_t chId,
                                     uint16_t timestamp,
                                     const std::vector<uint16_t>& samples)
{
  assertSync();

  addHeader(chId, samples);

  assertNofBits("timestamp", timestamp, 10);

  // append samples to bitset
  append10(static_cast<uint16_t>(samples.size()));
  append10(timestamp);

  for (auto s : samples) {
    append10(s);
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

  append10(nsamples);
  append10(timestamp);
  append20(chargeSum);
}

void ElinkEncoder::addChannelChargeSum(uint8_t chId,
                                       gsl::span<uint16_t> nsamples,
                                       gsl::span<uint16_t> timestamp,
                                       gsl::span<uint32_t> chargeSum)
{
  assertSync();

  addHeader(chId, nsamples, timestamp, chargeSum);

  // at this point we are sure the arrays are of the same size
  // and their content is legit
  for (int i = 0; i < nsamples.size(); i++) {
    append10(nsamples[i]);
    append10(timestamp[i]);
    append20(chargeSum[i]);
  }
}

void ElinkEncoder::addHeader(uint8_t chId,
                             gsl::span<uint16_t> nsamples,
                             gsl::span<uint16_t> timestamp,
                             gsl::span<uint32_t> chargeSum)
{
  if (nsamples.size() != timestamp.size() ||
      nsamples.size() != chargeSum.size() ||
      timestamp.size() != chargeSum.size()) {
    throw std::invalid_argument("nsamples,timestamp and chargeSum arrays must be of the same size");
  }
  int n10{0};
  for (int i = 0; i < nsamples.size(); i++) {
    assertNofBits("sample", nsamples[i], 10);
    assertNofBits("timestamp", timestamp[i], 10);
    assertNofBits("chargeSum", chargeSum[i], 20);
    n10 += 40;
  }
  setHeader(chId, n10);
  // append header to bitset
  append50(mSampaHeader.uint64());
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
  append50(mSampaHeader.uint64());
}

void ElinkEncoder::addHeader(uint8_t chId, uint32_t chargeSum)
{
  int n10 = 4; // nofsamples + timestamp + 20 bits of chargeSum
  assertNofBits("chargeSum", chargeSum, 20);
  setHeader(chId, n10);
  // append header to bitset
  append50(mSampaHeader.uint64());
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
  computeHamming(mSampaHeader);
}

std::ostream& operator<<(std::ostream& os, const ElinkEncoder& enc)
{
  os << fmt::sprintf("ELINK ID %2d chip %2d nsync %3llu len %6llu syncindex %2d nbitseen %10d | %s",
                     enc.mId, enc.mChipAddress, enc.mNofSync, enc.mBitSet.len(),
                     enc.mSyncIndex, enc.mNofBitSeen,
                     compactString(enc.mBitSet));
  return os;
}

} // namespace o2::mch::raw
