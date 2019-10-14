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

namespace o2
{
namespace mch
{
namespace raw
{

//FIXME: probably needs the GBT id as well here...
ElinkDecoder::ElinkDecoder(uint8_t cruId,
                           uint8_t linkId,
                           SampaChannelHandler sampaChannelHandler,
                           bool chargeSumMode) : mCruId{cruId},
                                                 mLinkId{linkId},
                                                 mCheckpoint(HEADERSIZE),
                                                 mIsInData(false),
                                                 mNofSync(0),
                                                 mBitSet(),
                                                 mSampaHeader(0),
                                                 mNofBitSeen(0),
                                                 mNofHeaderSeen(0),
                                                 mSampaChannelHandler{sampaChannelHandler},
                                                 mChargeSumMode{chargeSumMode},
                                                 mIsSynchronized{false}
{
  assertIsInRange("linkId", linkId, 0, 39);
}

bool ElinkDecoder::append(bool bit)
{
  ++mNofBitSeen;
  try {
    mBitSet.append(bit);
  } catch (std::length_error) {
    std::cout << (*this) << "\n";
    std::cout << "OUSP !!!\n";
    throw;
  }
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

/// findSync checks if the last 50 bits of the bit stream
/// match the Sampa SYNC word.
///
/// - if they are then reset the bit stream and sets the checkpoint to 50 bits
/// - if they are not then pop the first bit out
void ElinkDecoder::findSync()
{
  if (mIsSynchronized) {
    throw std::logic_error("wrong logic 2");
  }
  if (mIsInData) {
    throw std::logic_error("wrong logic 3");
  }
  mSampaHeader.uint64(mBitSet.last(HEADERSIZE).uint64(0, 49));
  if (mSampaHeader != sampaSync()) {
    mBitSet.pruneFirst(1);
    return;
  }
  clear(HEADERSIZE);
  mNofSync++;
  mIsSynchronized = true;
}

bool ElinkDecoder::getData()
{
  if (mSampaChannelHandler) {
    if (mChargeSumMode) {
      handlePacket20();
    } else {
      handlePacket10();
    }
  }
  softReset();
  return true;
}

void ElinkDecoder::handlePacket10()
{
  // handling sampa packet
  // for 10-bits samples case.
  int index{0};
  while (index < len()) {
    uint16_t nsamples = mBitSet.uint16(index, index + 9);
    index += 10;
    uint16_t timestamp = mBitSet.uint16(index, index + 9);
    index += 10;
    std::vector<uint16_t> samples;
    samples.reserve(nsamples);
    for (int i = 0; i < nsamples; i++) {
      samples.emplace_back(mBitSet.uint16(index, index + 9));
      index += 10;
    }
    mSampaChannelHandler(mCruId,
                         mLinkId,
                         mSampaHeader.chipAddress(),
                         mSampaHeader.channelAddress(),
                         SampaCluster(timestamp, samples));
  }
}

void ElinkDecoder::handlePacket20()
{
  // handling sampa packet
  // for 20-bits chargeSum case.
  int index{0};
  while (index < len()) {
    uint16_t nsamples = mBitSet.uint16(index, index + 9);
    index += 10;
    uint16_t timestamp = mBitSet.uint16(index, index + 9);
    index += 10;
    uint32_t chargeSum = mBitSet.uint32(index, index + 19);
    mSampaChannelHandler(mCruId,
                         mLinkId,
                         mSampaHeader.chipAddress(),
                         mSampaHeader.channelAddress(),
                         SampaCluster(timestamp, chargeSum));
    index += 20;
  }
} // namespace raw

int ElinkDecoder::len() const
{
  return mBitSet.len();
}

uint8_t ElinkDecoder::linkId() const
{
  return mLinkId;
}

/// process the bit stream content.
///
/// can be in basically 3 states when entering this method :
///
/// - mIsSynchronized=false : we still have to find our first SYNC word
/// - mIsInData=true : we have reached a data "zone", so we read the data
/// - none of the above : we are looking for a header (50 bits). Depending
/// on the value of the header type, we change state.
///
bool ElinkDecoder::process()
{
  if (mBitSet.len() != mCheckpoint) {
    throw std::logic_error("wrong logic somewhere");
  }

  // first things first : we must find the sync pattern, otherwise
  // just continue
  if (!mIsSynchronized) {
    findSync();
    return true;
  }

  // we have reached a point where we do have data, let's decode it
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
    case SampaPacketType::DataNumWords:
      // data with a problem is still data, i.e. there will
      // probably be some data words to read in, unless there's none
    case SampaPacketType::Data:
      clear(10 * mSampaHeader.nof10BitWords());
      mIsInData = true;
      return true;
      break;
    case SampaPacketType::Sync:
      mNofSync++;
      softReset();
      return true;
      break;
    case SampaPacketType::HeartBeat:
      fmt::printf("ElinkDecoder %d: HEARTBEAT found. Should be doing sth about it ?\n", mLinkId);
      softReset();
      return true;
      break;
    default:
      throw std::logic_error("that should not be possible");
      break;
  }
  return true;
}

void ElinkDecoder::softReset()
{
  clear(HEADERSIZE);
  mIsInData = false;
}

void ElinkDecoder::hardReset()
{
  softReset();
  mIsSynchronized = false;
}

std::ostream& operator<<(std::ostream& os, const ElinkDecoder& e)
{
  os << fmt::format("ELINK ID {:2d} cruId {:2d} nsync {:6d} checkpoint {:6d} indata {:1d} len {:6d} nbits seen {:6d} headers {:6d} maxlen {:6d} mode {} {}",
                    e.mLinkId, e.mCruId, e.mNofSync, e.mCheckpoint,
                    e.mIsInData, e.mBitSet.len(), e.mNofBitSeen,
                    e.mNofHeaderSeen, e.mBitSet.maxlen(),
                    (e.mChargeSumMode ? "CLUSUM" : "SAMPLE"),
                    (e.len() == BitSet::maxSize() ? "FULL!!!" : ""));

  // if (e.len()) {
  //   os << std::string(6, ' ') << "BitSet=" << compactString(e.mBitSet);
  // }
  return os;
}

} // namespace raw
} // namespace mch
} // namespace o2
