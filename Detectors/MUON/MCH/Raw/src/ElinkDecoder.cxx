// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHRaw/ElinkDecoder.h"
#include <stdexcept>
#include <fmt/format.h>
#include <fmt/printf.h>
#include "CompactBitSetString.h"
#include "Assertions.h"

constexpr int HEADERSIZE = 50;

namespace
{
std::string bitBufferString(const std::bitset<50>& bs, int imax)
{
  std::string s;
  for (int i = 0; i < imax; i++) {
    if (bs.test(i)) {
      s += "1";
    } else {
      s += "0";
    }
  }
  return s;
}
} // namespace

namespace o2
{
namespace mch
{
namespace raw
{

//FIXME: probably needs the GBT id as well here ?
ElinkDecoder::ElinkDecoder(uint8_t cruId,
                           uint8_t linkId,
                           SampaChannelHandler sampaChannelHandler,
                           bool chargeSumMode) : mCruId{cruId},
                                                 mLinkId{linkId},
                                                 mSampaChannelHandler{sampaChannelHandler},
                                                 mSampaHeader{},
                                                 mClusterSumMode{chargeSumMode},
                                                 mBitBuffer{},
                                                 mBitBufferIndex{},
                                                 mNofSync{},
                                                 mNofBitSeen{},
                                                 mNofHeaderSeen{},
                                                 mCheckpoint{HEADERSIZE},
                                                 mNof10BitsWordsToRead{},
                                                 mNofSamples{},
                                                 mTimestamp{},
                                                 mSamples{},
                                                 mClusterSum{},
                                                 mState{State::LookingForSync}
{
  assertIsInRange("linkId", linkId, 0, 39);
}

void ElinkDecoder::append(bool bit)
{
  constexpr uint64_t one{1};

  ++mNofBitSeen;

  if (bit) {
    mBitBuffer |= one << mBitBufferIndex;
  } else {
    mBitBuffer &= ~(one << mBitBufferIndex);
  }

  ++mBitBufferIndex;

  if (mBitBufferIndex == mCheckpoint) {
    process();
  }
}

void ElinkDecoder::append(bool bit0, bool bit1)
{
  append(bit0);
  append(bit1);
}

void ElinkDecoder::changeState(State newState, int newCheckpoint)
{
  mState = newState;
  clear(newCheckpoint);
}

void ElinkDecoder::clear(int checkpoint)
{
  mBitBufferIndex = 0;
  mBitBuffer = 0;
  mCheckpoint = checkpoint;
}

/// findSync checks if the last 50 bits of the bit stream
/// match the Sampa SYNC word.
///
/// - if they are then reset the bit stream and sets the checkpoint to 50 bits
/// - if they are not then pop the first bit out
void ElinkDecoder::findSync()
{
  assert(mState == State::LookingForSync);
  const uint64_t sync = sampaSync().uint64();
  if (mBitBuffer != sync) {
    mBitBuffer >>= 1;
    mBitBufferIndex--;
    return;
  }
  changeState(State::LookingForHeader, HEADERSIZE);
  mNofSync++;
}

void ElinkDecoder::handleHeader()
{
  assert(mState == State::LookingForHeader);

  mSampaHeader.uint64(mBitBuffer);
  ++mNofHeaderSeen;

  switch (mSampaHeader.packetType()) {
    case SampaPacketType::DataTruncated:
    case SampaPacketType::DataTruncatedTriggerTooEarly:
    case SampaPacketType::DataTriggerTooEarly:
    case SampaPacketType::DataTriggerTooEarlyNumWords:
    case SampaPacketType::DataNumWords:
      // data with a problem is still data, i.e. there will
      // probably be some data words to read in...
      // so we fallthrough the simple Data case
    case SampaPacketType::Data:
      mNof10BitsWordsToRead = mSampaHeader.nof10BitWords();
      changeState(State::ReadingNofSamples, 10);
      break;
    case SampaPacketType::Sync:
      mNofSync++;
      softReset();
      break;
    case SampaPacketType::HeartBeat:
      fmt::printf("ElinkDecoder %d: HEARTBEAT found. Should be doing sth about it ?\n", mLinkId);
      softReset();
      break;
    default:
      throw std::logic_error("that should not be possible");
      break;
  }
}

void ElinkDecoder::handleReadClusterSum()
{
  mClusterSum = mBitBuffer;
  oneLess10BitWord();
  oneLess10BitWord();
  sendCluster();
  if (mNof10BitsWordsToRead) {
    changeState(State::ReadingNofSamples, 10);
  } else {
    changeState(State::LookingForHeader, HEADERSIZE);
  }
}

void ElinkDecoder::handleReadData()
{
  assert(mState == State::ReadingTimestamp || mState == State::ReadingSample);
  if (mState == State::ReadingTimestamp) {
    mTimestamp = mBitBuffer;
  }
  oneLess10BitWord();
  if (mClusterSumMode) {
    changeState(State::ReadingClusterSum, 20);
  } else {
    changeState(State::ReadingSample, 10);
  }
}

void ElinkDecoder::handleReadSample()
{
  mSamples.push_back(mBitBuffer);
  if (mNofSamples > 0) {
    --mNofSamples;
  }
  oneLess10BitWord();
  if (mNofSamples) {
    handleReadData();
  } else {
    sendCluster();
    if (mNof10BitsWordsToRead) {
      changeState(State::ReadingNofSamples, 10);
    } else {
      changeState(State::LookingForHeader, HEADERSIZE);
    }
  }
}

void ElinkDecoder::handleReadTimestamp()
{
  assert(mState == State::ReadingNofSamples);
  oneLess10BitWord();
  mNofSamples = mBitBuffer;
  changeState(State::ReadingTimestamp, 10);
}

int ElinkDecoder::len() const
{
  return mBitBufferIndex;
}

uint8_t ElinkDecoder::linkId() const
{
  return mLinkId;
}

std::string ElinkDecoder::name(State s) const
{
  switch (s) {
    case State::LookingForSync:
      return "LookingForSync";
      break;
    case State::LookingForHeader:
      return "LookingForHeader";
      break;
    case State::ReadingNofSamples:
      return "ReadingNofSamples";
      break;
    case State::ReadingTimestamp:
      return "ReadingTimestamp";
      break;
    case State::ReadingSample:
      return "ReadingSample";
      break;
    case State::ReadingClusterSum:
      return "ReadingClusterSum";
      break;
  };
}

void ElinkDecoder::oneLess10BitWord()
{
  if (mNof10BitsWordsToRead > 0) {
    --mNof10BitsWordsToRead;
  }
}

/// process the bit stream content.
void ElinkDecoder::process()
{
  if (len() != mCheckpoint) {
    throw std::logic_error("wrong logic somewhere");
  }

  switch (mState) {
    case State::LookingForSync:
      findSync();
      break;
    case State::LookingForHeader:
      handleHeader();
      break;
    case State::ReadingNofSamples:
      handleReadTimestamp();
      break;
    case State::ReadingTimestamp:
      handleReadData();
      break;
    case State::ReadingSample:
      handleReadSample();
      break;
    case State::ReadingClusterSum:
      handleReadClusterSum();
      break;
  }
};

void ElinkDecoder::sendCluster()
{
  if (!mSampaChannelHandler) {
    // to nothing if we don't have a channel handler
    return;
  }
  if (mClusterSumMode) {
    mSampaChannelHandler(mCruId,
                         mLinkId,
                         mSampaHeader.chipAddress(),
                         mSampaHeader.channelAddress(),
                         SampaCluster(mTimestamp, mClusterSum));
  } else {
    mSampaChannelHandler(mCruId,
                         mLinkId,
                         mSampaHeader.chipAddress(),
                         mSampaHeader.channelAddress(),
                         SampaCluster(mTimestamp, mSamples));
  }
  mSamples.clear();
}

void ElinkDecoder::softReset()
{
  clear(HEADERSIZE);
}

void ElinkDecoder::reset()
{
  softReset();
  mState = State::LookingForSync;
}

std::ostream& operator<<(std::ostream& os, const ElinkDecoder& e)
{
  os << fmt::format("ID{:2d} cruId {:2d} sync {:6d} cp {:6d} bi {:2d} state {:17s} len {:6d} nseen {:6d} head {:6d} n10w {:6d} nsamples {:6d} mode {} bbuf {:s}",
                    e.mLinkId, e.mCruId, e.mNofSync, e.mCheckpoint, e.mBitBufferIndex,
                    e.name(e.mState),
                    e.len(), e.mNofBitSeen,
                    e.mNofHeaderSeen,
                    e.mNof10BitsWordsToRead,
                    e.mNofSamples,
                    (e.mClusterSumMode ? "CLUSUM" : "SAMPLE"),
                    bitBufferString(e.mBitBuffer, e.mBitBufferIndex));
  return os;
}

} // namespace raw
} // namespace mch
} // namespace o2
