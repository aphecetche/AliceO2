// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHRaw/ELink.h"
#include <stdexcept>
#include <fmt/printf.h>

namespace
{
constexpr int HEADERSIZE = 50;
}
namespace o2::mch::raw
{

std::ostream& operator<<(std::ostream& os, const ELink& e)
{
  os << fmt::sprintf("ELINK ID %d nsync %d checkpoint %d indata %d len %d nbits seen %llu headers %llu\n",
                     e.mId, e.mNofSync, e.mCheckpoint, e.mIsInData, e.mBitSet.len(), e.mNofBitSeen, e.mNofHeaderSeen);
  return os;
}

ELink::ELink(int id) : mId{id}, mCheckpoint(HEADERSIZE), mIsInData(false), mNofSync(0), mBitSet(), mSampaHeader(0), mNofBitSeen(0), mNofHeaderSeen(0)
{
}

bool ELink::append(bool bit)
{
  ++mNofBitSeen;
  mBitSet.append(bit);
  if (mBitSet.len() != mCheckpoint) {
    return true;
  }
  return process();
}

bool ELink::append(bool bit0, bool bit1)
{
  return append(bit0) && append(bit1);
}

void ELink::clear(int checkpoint)
{
  mBitSet.clear();
  mCheckpoint = checkpoint;
}

void ELink::findSync()
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
bool ELink::process()
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
      fmt::printf("ELink %d: HEARTBEAT found. Should be doing sth about it ?\n", mId);
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

void ELink::getPacket()
{
  // here we should get chuncks of 10 bits.
  // a sampa packet is (header+payload) for one channel

  // up to checkpoint bits should be our data

  std::cout << "FIXME: write getPacket checkpoint=" << mCheckpoint << "\n";
  show(mBitSet, 0, 9);
  show(mBitSet, 10, 19);
  show(mBitSet, 20, 39);
}

} // namespace o2::mch::raw
