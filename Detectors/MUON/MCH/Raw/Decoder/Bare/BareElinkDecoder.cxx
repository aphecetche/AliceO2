// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "BareElinkDecoder.h"
#include <stdexcept>
#include <fmt/format.h>
#include <fmt/printf.h>
#include "Assertions.h"
#include <bitset>

constexpr int HEADERSIZE = 50;

namespace
{
std::string bitBufferString(const std::bitset<50>& bs, int imax)
{
  std::string s;
  for (int i = 0; i < 64; i++) {
    if ((static_cast<uint64_t>(1) << i) > imax) {
      break;
    }
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
BareElinkDecoder::BareElinkDecoder(uint8_t cruId,
                                   uint8_t linkId,
                                   SampaChannelHandler sampaChannelHandler,
                                   bool chargeSumMode) : mCruId{cruId},
                                                         mLinkId{linkId},
                                                         mSampaChannelHandler{sampaChannelHandler},
                                                         mSampaHeader{},
                                                         mClusterSumMode{chargeSumMode},
                                                         mBitBuffer{},
                                                         mNofSync{},
                                                         mNofBitSeen{},
                                                         mNofHeaderSeen{},
                                                         mNofHammingErrors{},
                                                         mNofHeaderParityErrors{},
                                                         mCheckpoint{(static_cast<uint64_t>(1) << HEADERSIZE)},
                                                         mNof10BitsWordsToRead{},
                                                         mNofSamples{},
                                                         mTimestamp{},
                                                         mSamples{},
                                                         mClusterSum{},
                                                         mState{State::LookingForSync},
                                                         mMask{1}
{
  assertIsInRange("linkId", linkId, 0, 39);
}

void BareElinkDecoder::append(bool bit0, bool bit1)
{
  mNofBitSeen += 2;

  mBitBuffer += bit0 * mMask + bit1 * mMask * 2;
  mMask *= 4;

  if (mMask == mCheckpoint) {
    process();
  }
}

void BareElinkDecoder::changeState(State newState, int newCheckpoint)
{
  mState = newState;
  clear(newCheckpoint);
}

void BareElinkDecoder::clear(int checkpoint)
{
  mBitBuffer = 0;
  mCheckpoint = static_cast<uint64_t>(1) << checkpoint;
  mMask = 1;
}

/// findSync checks if the last 50 bits of the bit stream
/// match the Sampa SYNC word.
///
/// - if they are then reset the bit stream and sets the checkpoint to 50 bits
/// - if they are not then pop the first bit out
void BareElinkDecoder::findSync()
{
  assert(mState == State::LookingForSync);
  const uint64_t sync = sampaSync().uint64();
  if (mBitBuffer != sync) {
    mBitBuffer >>= 1;
    mMask /= 2;
    return;
  }
  changeState(State::LookingForHeader, HEADERSIZE);
  mNofSync++;
}

void BareElinkDecoder::handleHeader()
{
  assert(mState == State::LookingForHeader);

  mSampaHeader.uint64(mBitBuffer);
  ++mNofHeaderSeen;

  if (mSampaHeader.hasError()) {
    ++mNofHammingErrors;
  }

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
      fmt::printf("BareElinkDecoder %d: HEARTBEAT found. Should be doing sth about it ?\n", mLinkId);
      softReset();
      break;
    default:
      throw std::logic_error("that should not be possible");
      break;
  }
}

void BareElinkDecoder::handleReadClusterSum()
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

void BareElinkDecoder::handleReadData()
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

void BareElinkDecoder::handleReadSample()
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

void BareElinkDecoder::handleReadTimestamp()
{
  assert(mState == State::ReadingNofSamples);
  oneLess10BitWord();
  mNofSamples = mBitBuffer;
  changeState(State::ReadingTimestamp, 10);
}

int BareElinkDecoder::len() const
{
  return static_cast<int>(std::floor(log2(1.0 * mMask)) + 1);
}

uint8_t BareElinkDecoder::linkId() const
{
  return mLinkId;
}

std::string BareElinkDecoder::name(State s) const
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

void BareElinkDecoder::oneLess10BitWord()
{
  if (mNof10BitsWordsToRead > 0) {
    --mNof10BitsWordsToRead;
  }
}

/// process the bit stream content.
void BareElinkDecoder::process()
{
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

void BareElinkDecoder::sendCluster()
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

void BareElinkDecoder::softReset()
{
  clear(HEADERSIZE);
}

void BareElinkDecoder::reset()
{
  softReset();
  mState = State::LookingForSync;
}

std::ostream& operator<<(std::ostream& os, const BareElinkDecoder& e)
{
  os << fmt::format("ID{:2d} cruId {:2d} sync {:6d} cp 0x{:6x} mask 0x{:6x} state {:17s} len {:6d} nseen {:6d} errH {:6} errP {:6} head {:6d} n10w {:6d} nsamples {:6d} mode {} bbuf {:s}",
                    e.mLinkId, e.mCruId, e.mNofSync, e.mCheckpoint, e.mMask,
                    e.name(e.mState),
                    e.len(), e.mNofBitSeen,
                    e.mNofHeaderSeen,
                    e.mNofHammingErrors,
                    e.mNofHeaderParityErrors,
                    e.mNof10BitsWordsToRead,
                    e.mNofSamples,
                    (e.mClusterSumMode ? "CLUSUM" : "SAMPLE"),
                    bitBufferString(e.mBitBuffer, e.mMask));
  return os;
}

} // namespace raw
} // namespace mch
} // namespace o2
