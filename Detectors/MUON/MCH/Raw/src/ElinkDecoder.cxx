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

namespace
{
constexpr int HEADERSIZE = 50;
}
namespace o2::mch::raw
{

std::ostream& operator<<(std::ostream& os, const ElinkDecoder& e)
{
  os << fmt::format("ELINK ID {} nsync {} checkpoint {} indata {} len {} nbits seen {} headers {}\n",
                    e.mId, e.mNofSync, e.mCheckpoint, e.mIsInData, e.mBitSet.len(), e.mNofBitSeen, e.mNofHeaderSeen);

  if (e.len()) {
    os << std::string(6, ' ') << "BitSet=" << compactString(e.mBitSet);
  }
  return os;
}

ElinkDecoder::ElinkDecoder(uint8_t id, SampaChannelHandler sampaChannelHandler) : mId{id}, mCheckpoint(HEADERSIZE), mIsInData(false), mNofSync(0), mBitSet(), mSampaHeader(0), mNofBitSeen(0), mNofHeaderSeen(0), mSampaChannelHandler(sampaChannelHandler)
{
  if (id > 39) {
    throw std::invalid_argument(fmt::sprintf("id = %d should be between 0 and 39", id));
  }
}

bool ElinkDecoder::append(bool bit)
{
  ++mNofBitSeen;
  mBitSet.append(bit);
  if (mBitSet.len() != mCheckpoint) {
    return true;
  }
  return process();
}

bool ElinkDecoder::append(bool bit0, bool bit1)
{
  return append(bit0) && append(bit1);
}

void ElinkDecoder::clear(int checkpoint)
{
  mBitSet.clear();
  mCheckpoint = checkpoint;
}

bool ElinkDecoder::finalize()
{
  if (mIsInData) {
    return getData();
  }
  return false;
}

void ElinkDecoder::findSync()
{
  if (mNofSync != 0) {
    throw std::logic_error("wrong logic 2");
  }
  if (mIsInData) {
    throw std::logic_error("wrong logic 3");
  }
  mSampaHeader.uint64(mBitSet.last(HEADERSIZE).uint64(0, 49));
  if (mSampaHeader != sampaSync()) {
    mCheckpoint++;
    return;
  }
  clear(HEADERSIZE);
  mNofSync++;
}

bool ElinkDecoder::getData()
{
  getPacket();
  mBitSet.clear();
  mCheckpoint = HEADERSIZE;
  mIsInData = false;
  return true;
}

void ElinkDecoder::getPacket()
{
  // FIXME: should get the information here whether
  // we have a packet consisting of 10-bits data sample
  // or 20-bits chargeSum...
  //
  // assuming 20-bits chargeSum for the moment.
  //
  //uint16_t nsamples = mBitSet.uint16(0,9);
  uint16_t timestamp = mBitSet.uint16(10, 19);
  uint32_t chargeSum = mBitSet.uint32(20, 39);
  mSampaChannelHandler(mSampaHeader.chipAddress(),
                       mSampaHeader.channelAddress(),
                       timestamp,
                       chargeSum);
}

int ElinkDecoder::len() const
{
  return mBitSet.len();
}

// process attempts to interpret the current bitset
// as either a Sampa Header or Sampa data block.
// If it's neither, then set the checkpoint at the current length
// plus two bits.
bool ElinkDecoder::process()
{
  if (mBitSet.len() != mCheckpoint) {
    throw std::logic_error("wrong logic somewhere");
  }

  // first things first : we must find the sync pattern, otherwise
  // just continue
  if (mNofSync == 0) {
    findSync();
    return true;
  }

  if (mIsInData) {
    return getData();
  }

  // looking for a header
  if (mCheckpoint != HEADERSIZE) {
    throw std::logic_error(fmt::sprintf("wrong logic 5 checkpoint %d HeaderSize %d", mCheckpoint, HEADERSIZE));
  }

  mSampaHeader.uint64(mBitSet.last(HEADERSIZE).uint64(0, 49));
  ++mNofHeaderSeen;

  switch (mSampaHeader.packetType()) {
    case SampaPacketType::DataTruncated:
    case SampaPacketType::DataTruncatedTriggerTooEarly:
    case SampaPacketType::DataTriggerTooEarly:
    case SampaPacketType::DataTriggerTooEarlyNumWords:
      // data with a problem is still data, i.e. there will
      // probably be some data words to read in
    case SampaPacketType::Data:
      clear(10 * mSampaHeader.nof10BitWords());
      mIsInData = true;
      return true;
      break;
    case SampaPacketType::Sync:
      mNofSync++;
      clear(HEADERSIZE);
      return true;
      break;
    case SampaPacketType::HeartBeat:
      fmt::printf("ElinkDecoder %d: HEARTBEAT found. Should be doing sth about it ?\n", mId);
      clear(HEADERSIZE);
      return true;
      break;
    default:
      std::cout << mSampaHeader << "\n";
      throw std::logic_error("that should not be possible");
      break;
  }
  return true;
}

} // namespace o2::mch::raw
