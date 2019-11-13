// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_USER_LOGIC_ELINK_ENCODER_H
#define O2_MCH_RAW_USER_LOGIC_ELINK_ENCODER_H

#include "ElinkEncoder.h"
#include "Assertions.h"
#include "MCHRawCommon/SampaCluster.h"
#include "MCHRawCommon/SampaHeader.h"
#include "MCHRawCommon/DataFormats.h"
#include "MoveBuffer.h"
#include "NofBits.h"
#include <cstdlib>
#include <vector>

namespace o2::mch::raw
{

template <typename CHARGESUM>
class ElinkEncoder<UserLogicFormat, CHARGESUM>
{
 public:
  explicit ElinkEncoder(uint8_t elinkId, uint8_t chip, int phase = 0);

  void addChannelData(uint8_t chId, const std::vector<SampaCluster>& data);

  size_t moveToBuffer(std::vector<uint64_t>& buffer, uint64_t prefix);

  void clear();

 private:
  uint8_t mElinkId;     //< Elink id 0..39
  uint8_t mChipAddress; //< chip address 0..15
  bool mHasSync;        //< whether or not we've already added a sync word
  std::vector<uint64_t> mBuffer;
};

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

template <typename CHARGESUM>
ElinkEncoder<UserLogicFormat, CHARGESUM>::ElinkEncoder(uint8_t elinkId,
                                                       uint8_t chip,
                                                       int phase)
  : mElinkId{elinkId},
    mChipAddress{chip},
    mHasSync{false},
    mBuffer{}
{
  impl::assertIsInRange("elinkId", elinkId, 0, 39);
  impl::assertIsInRange("chip", chip, 0, 15);
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

template <typename CHARGESUM>
void ElinkEncoder<UserLogicFormat, CHARGESUM>::addChannelData(uint8_t chId,
                                                              const std::vector<SampaCluster>& data)
{
  impl::assertNofBits("chId", chId, 5);
  if (data.empty()) {
    throw std::invalid_argument("cannot add empty data");
  }
  assertNotMixingClusters<CHARGESUM>(data);

  int error{0}; // FIXME: what to do with error ?

  uint64_t b9 = dsId64(mElinkId) | error64(error);

  const uint64_t sync = sampaSync().uint64();

  if (!mHasSync) {
    mBuffer.emplace_back(b9 | sync);
    mHasSync = true;
  }

  mBuffer.emplace_back(b9 | headerWord(mElinkId, chId, data));

  int index{4};
  CHARGESUM ref;
  uint64_t word{0};
  for (auto& cluster : data) {
    append(b9, mBuffer, index, word, cluster.nofSamples());
    append(b9, mBuffer, index, word, cluster.timestamp);
    if (ref() == true) {
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
template <typename CHARGESUM>
void ElinkEncoder<UserLogicFormat, CHARGESUM>::clear()
{
  mBuffer.clear();
  mHasSync = false;
}

template <typename CHARGESUM>
size_t ElinkEncoder<UserLogicFormat, CHARGESUM>::moveToBuffer(std::vector<uint64_t>& buffer, uint64_t prefix)
{
  mHasSync = false;
  auto n = buffer.size();
  for (auto& b : mBuffer) {
    buffer.emplace_back(b | prefix);
  }
  return buffer.size() - n;
}
} // namespace o2::mch::raw

#endif
