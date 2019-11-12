// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "UserLogicElinkEncoder.h"
#include "MCHRawCommon/SampaHeader.h"
#include "Assertions.h"
#include "NofBits.h"
#include "MoveBuffer.h"

namespace
{
uint64_t dsId64(int dsId)
{
  return (static_cast<uint64_t>(dsId & 0x3F) << 53);
}

uint64_t error64(int error)
{
  return (static_cast<uint64_t>(error & 0x7) << 50);
}
} // namespace

namespace o2::mch::raw
{

UserLogicElinkEncoder::UserLogicElinkEncoder(uint8_t elinkId,
                                             uint8_t chip,
                                             int phase,
                                             bool chargeSumMode)
  : mElinkId{elinkId},
    mChipAddress{chip},
    mChargeSumMode{chargeSumMode},
    mHasSync{false},
    mBuffer{}
{
  assertIsInRange("elinkId", elinkId, 0, 39);
  assertIsInRange("chip", chip, 0, 15);
}

uint16_t chipAddress(int elinkId, int chId)
{
  uint16_t chip = static_cast<uint16_t>((elinkId % 2) * 2);
  if (chId < 32) {
    return chip;
  }
  return chip + 1;
}

uint64_t headerWord(int elinkId, int chId, const std::vector<SampaCluster>& data)
{
  SampaHeader header;
  header.packetType(SampaPacketType::Data);
  int numWords{0};
  for (auto c : data) {
    numWords += 20 + 10 * c.samples.size(); // FIXME: assuming we're not in chargesum here
  }
  header.nof10BitWords(numWords);
  header.chipAddress(chipAddress(elinkId, chId));
  header.channelAddress(chId % 32);
  // FIXME: compute payload parity
  // FIXME: set local bunch crossing number
  // header.bunchCrossingCounter();
  return header.uint64();
}

void append(uint64_t prefix, std::vector<uint64_t>& buffer, int& index, uint64_t& word, int data)
{
  word |= static_cast<uint64_t>(data) << (index * 10);
  --index;
  if (index < 0) {
    buffer.emplace_back(prefix | word);
    index = 4;
  }
}

void UserLogicElinkEncoder::addChannelData(uint8_t chId,
                                           const std::vector<SampaCluster>& data)
{
  assertNofBits("chId", chId, 5);
  if (data.empty()) {
    throw std::invalid_argument("cannot add empty data");
  }
  assertNotMixingClusters(data, mChargeSumMode);

  int error{0}; // FIXME: what to do with error ?

  uint64_t b9 = dsId64(mElinkId) | error64(error);

  const uint64_t sync = sampaSync().uint64();

  if (!mHasSync) {
    mBuffer.emplace_back(b9 | sync);
    mHasSync = true;
  }

  mBuffer.emplace_back(b9 | headerWord(mElinkId, chId, data));

  int index{4};
  uint64_t word{0};
  for (auto& cluster : data) {
    append(b9, mBuffer, index, word, cluster.nofSamples());
    append(b9, mBuffer, index, word, cluster.timestamp);
    if (mChargeSumMode == true) {
      append(b9, mBuffer, index, word, cluster.chargeSum & 0x3FF);
      append(b9, mBuffer, index, word, (cluster.chargeSum & 0xFFC00) >> 10);
    } else {
      for (auto& s : cluster.samples) {
        append(b9, mBuffer, index, word, s);
      }
    }
  }
  while (index != 4) {
    append(b9, mBuffer, index, word, 0);
  }
}

size_t UserLogicElinkEncoder::moveToBuffer(std::vector<uint8_t>& buffer, uint64_t prefix)
{
  mHasSync = false;
  return moveBuffer(mBuffer, buffer, prefix);
}
} // namespace o2::mch::raw
