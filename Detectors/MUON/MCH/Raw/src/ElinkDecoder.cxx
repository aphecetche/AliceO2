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
#include <fmt/printf.h>

namespace
{
constexpr int HEADERSIZE = 50;
}
namespace o2::mch::raw
{

std::ostream& operator<<(std::ostream& os, const ElinkDecoder& e)
{
  os << fmt::sprintf("ELINK ID %d nsync %d checkpoint %d indata %d len %d nbits seen %llu headers %llu\n",
                     e.mId, e.mNofSync, e.mCheckpoint, e.mIsInData, e.mBitSet.len(), e.mNofBitSeen, e.mNofHeaderSeen);
  return os;
}

ElinkDecoder::ElinkDecoder(int id, PacketHandler packetHandler) : mId{id}, mCheckpoint(HEADERSIZE), mIsInData(false), mNofSync(0), mBitSet(), mSampaHeader(0), mNofBitSeen(0), mNofHeaderSeen(0), mPacketHandler(packetHandler)
{
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

void ElinkDecoder::findSync()
{
  if (mNofSync != 0) {
    throw std::logic_error("wrong logic 2");
  }
  if (mIsInData) {
    throw std::logic_error("wrong logic 3");
  }
  mSampaHeader = SampaHeader(mBitSet.last(HEADERSIZE).uint64(0, 49));
  if (mSampaHeader != sampaSync()) {
    mCheckpoint++;
    return;
  }
  std::cout << "Found SYNC " << (*this) << "\n";
  std::cout << mSampaHeader << "\n";
  clear(HEADERSIZE);
  mNofSync++;
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
    // data mode, just decode ourselves into a set of sampa packets
    getPacket();
    mBitSet.clear();
    mCheckpoint = HEADERSIZE;
    mIsInData = false;
    return true;
  }

  // looking for a header
  if (mCheckpoint != HEADERSIZE) {
    throw std::logic_error(fmt::sprintf("wrong logic 5 checkpoint %d HeaderSize %d", mCheckpoint, HEADERSIZE));
  }

  mSampaHeader = SampaHeader(mBitSet.last(HEADERSIZE).uint64(0, 49));
  ++mNofHeaderSeen;

  std::cout << packetTypeName(mSampaHeader.packetType()) << "\n";

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

void show(const BitSet& bs, int a, int b)
{
  auto s = bs.subset(a, b);
  std::cout << s.stringLSBLeft() << " = " << s.uint32(0, b - a - 1) << "\n";
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
  mPacketHandler(mSampaHeader.chipAddress(),
                 mSampaHeader.channelAddress(),
                 timestamp,
                 chargeSum);
}

} // namespace o2::mch::raw
