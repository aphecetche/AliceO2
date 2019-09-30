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

namespace o2::mch::raw
{

void computeHamming(SampaHeader& sampaHeader)
{
  // FIXME: compute hamming and parities
}

ElinkEncoder::ElinkEncoder(uint8_t id, uint8_t dsid) : mId(id), mDsId(dsid), mSampaHeader{}, mBitSet{}
{
  if (id > 39) {
    throw std::invalid_argument(fmt::sprintf("id = %d should be between 0 and 39", id));
  }
  mSampaHeader.chipAddress(mDsId);
}

void assertNofBits(std::string_view msg, uint64_t value, int allowed)
{
  if (nofBits(value) > allowed) {
    throw std::invalid_argument(fmt::sprintf("%s=0x%x has %d bits, which is more than the %d allowed", msg, value, nofBits(value), allowed));
  }
}

void ElinkEncoder::bunchCrossingCounter(uint32_t bx)
{
  assertNofBits("bx", bx, 20);
  mSampaHeader.bunchCrossingCounter(bx);
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
}

void ElinkEncoder::addHeader(uint8_t chId, uint32_t chargeSum)
{
  int n10 = 4; // nofsamples + timestamp + 20 bits of chargeSum
  assertNofBits("chargeSum", chargeSum, 20);
  setHeader(chId, n10);
  // append header to bitset
  mBitSet.append(mSampaHeader.uint64(), 50);
}

void ElinkEncoder::addChannelSamples(uint8_t chId,
                                     uint16_t timestamp,
                                     const std::vector<uint16_t>& samples)
{
  addHeader(chId, samples);

  assertNofBits("timestamp", timestamp, 10);

  // append samples to bitset
  mBitSet.append(static_cast<uint16_t>(samples.size()), 10);
  mBitSet.append(timestamp, 10);

  for (auto s : samples) {
    mBitSet.append(s, 10);
  }
}

void ElinkEncoder::addChannelChargeSum(uint8_t chId,
                                       uint16_t timestamp,
                                       uint32_t chargeSum,
                                       uint16_t nsamples)
{
  addHeader(chId, chargeSum);

  // append samples to bitset
  assertNofBits("nsamples", nsamples, 10);
  assertNofBits("timestamp", timestamp, 10);

  mBitSet.append(nsamples, 10);
  mBitSet.append(timestamp, 10);
  mBitSet.append(chargeSum, 20);
}

void ElinkEncoder::addSync()
{
  mBitSet.append(sampaSync().uint64(), 50);
}

int ElinkEncoder::fillWithSync(int upto)
{
  BitSet sync{sampaSync().uint64()};
  int ix{0};
  int i{len()};
  int n{0};

  while (i < upto) {
    mBitSet.set(i, sync.get(ix));
    n++;
    i++;
    ix++;
    if (ix == 50) {
      ix = 0;
    }
  }
  return n;
}

uint64_t ElinkEncoder::range(int a, int b) const
{
  return mBitSet.subset(a, b).uint64(0, b - a + 1);
}

void ElinkEncoder::addRandomBits(int n)
{
  for (int i = 0; i < n; i++) {
    mBitSet.append(static_cast<bool>(rand() % 2));
  }
}

std::ostream& operator<<(std::ostream& os, const ElinkEncoder& enc)
{
  os << enc.mBitSet.stringLSBLeft();
  return os;
}

void ElinkEncoder::clear()
{
  mBitSet.clear();
}

bool ElinkEncoder::get(int i) const
{
  if (i >= len()) {
    throw std::invalid_argument("trying to access bit past the end of our bitset");
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
