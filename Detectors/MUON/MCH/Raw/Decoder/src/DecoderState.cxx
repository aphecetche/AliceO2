// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "DecoderState.h"
#include <sstream>
#include "Debug.h"

namespace o2::mch::raw
{

DecoderState::DecoderState(DsElecId dsId, SampaChannelHandler sampaChannelHandler)
  : mDsId{dsId}, mSampaChannelHandler{sampaChannelHandler}
{
}

DsElecId DecoderState::dsId() const
{
  return mDsId;
}

std::ostream& DecoderState::debugHeader() const
{
  std::cout << fmt::format("--ULDEBUG--{:s}-----------", asString(mDsId));
  return std::cout;
}

bool DecoderState::hasError() const
{
  return mErrorMessage.has_value();
}

std::string DecoderState::errorMessage() const
{
  return hasError() ? mErrorMessage.value() : "";
}

void DecoderState::decrementClusterSize()
{
  --mClusterSize;
}

void DecoderState::setClusterSize(uint16_t value)
{
  mClusterSize = value;
  int checkSize = mClusterSize + 2 - mSampaHeader.nof10BitWords();
  mErrorMessage = std::nullopt;
  if (checkSize) {
    if (checkSize < 0) {
      mErrorMessage = "cluster size smaller than nof10BitWords";
    } else if (checkSize > 0) {
      mErrorMessage = "cluster size bigger than nof10BitWords !!!";
    }
  }
#ifdef ULDEBUG
  debugHeader() << " -> size=" << mClusterSize << " maskIndex=" << mMaskIndex
                << " nof10BitWords=" << mSampaHeader.nof10BitWords()
                << " " << (hasError() ? "ERROR:" : "") << errorMessage() << "\n";
#endif
}

void DecoderState::setClusterTime(uint16_t value)
{
  mClusterTime = value;
#ifdef ULDEBUG
  debugHeader() << " -> time=" << mClusterTime << " maskIndex=" << mMaskIndex << "\n";
#endif
}

void DecoderState::reset()
{
#ifdef ULDEBUG
  debugHeader() << " -> reset\n";
#endif
  mMaskIndex = mMasks.size();
  mHeaderParts.clear();
  mClusterSize = 0;
  mNof10BitWords = 0;
  mErrorMessage = std::nullopt;
}

void DecoderState::setData(uint64_t data)
{
  mData = data;
  mMaskIndex = 0;
#ifdef ULDEBUG
  debugHeader() << fmt::format(">>>>> setData {:08X} maskIndex {} 10bits=", mData, mMaskIndex);
  for (int i = 0; i < mMasks.size(); i++) {
    std::cout << fmt::format("{:2d} ", data10(mData, i));
  }
  std::cout << "\n";
#endif
}

uint16_t DecoderState::data10(uint64_t value, size_t index) const
{
  if (index < 0 || index >= mMasks.size()) {
    std::cout << "-- ULDEBUG -- " << fmt::format("index {} is out of range\n", index);
    return 0;
  }
  uint64_t m = mMasks[index];
  return static_cast<uint16_t>((value & m) >> (index * 10) & 0x3FF);
}

uint16_t DecoderState::pop10()
{
  auto rv = data10(mData, mMaskIndex);
  mNof10BitWords = std::max(0, mNof10BitWords - 1);
  mMaskIndex = std::min(mMasks.size(), mMaskIndex + 1);
  return rv;
}

void DecoderState::addSample(uint16_t sample)
{
#ifdef ULDEBUG
  debugHeader() << "sample = " << sample << "\n";
#endif
  mSamples.emplace_back(sample);

  if (mClusterSize == 0) {
    // a mCluster is ready, send it
#ifdef ULDEBUG
    std::stringstream s;
    s << SampaCluster(mClusterTime, mSamples);
    debugHeader() << fmt::format(" calling channelHandler for {} ch {} = {}\n",
                                 asString(mDsId), channelNumber64(mSampaHeader), s.str());
#endif
    mSampaChannelHandler(mDsId, channelNumber64(mSampaHeader), SampaCluster(mClusterTime, mSamples));
    mSamples.clear();
  }
}

void DecoderState::completeHeader()
{
  uint64_t header{0};
  for (auto i = 0; i < mHeaderParts.size(); i++) {
    header += (static_cast<uint64_t>(mHeaderParts[i]) << (10 * i));
  }

  mSampaHeader = SampaHeader(header);
  mNof10BitWords = mSampaHeader.nof10BitWords();

#ifdef ULDEBUG
  debugHeader()
    << fmt::format(">>>>> completeHeader {:013X}\n", header)
    << "\n"
    << mSampaHeader << "\n";
#endif

  mHeaderParts.clear();
}

void DecoderState::addHeaderPart(uint16_t a)
{
  mHeaderParts.emplace_back(a);
#ifdef ULDEBUG
  debugHeader()
    << fmt::format(">>>>> readHeader {:08X}", a);
  for (auto h : mHeaderParts) {
    std::cout << fmt::format("{:4d} ", h);
  }
  std::cout << "\n";
#endif
}

void DecoderState::addChargeSum(uint16_t b, uint16_t a)
{
  // a mCluster is ready, send it
  uint32_t q = (((static_cast<uint32_t>(a) & 0x3FF) << 10) | (static_cast<uint32_t>(b) & 0x3FF));
#ifdef ULDEBUG
  debugHeader()
    << "chargeSum = " << q << "\n";
#endif
  mSampaChannelHandler(mDsId,
                       channelNumber64(mSampaHeader),
                       SampaCluster(mClusterTime, q));
}

bool DecoderState::moreDataAvailable() const
{
  bool rv = mMaskIndex < mMasks.size();
#ifdef ULDEBUG
  debugHeader() << fmt::format("moreDataAvailable {} maskIndex {}\n", rv, mMaskIndex);
#endif
  return rv;
}

bool DecoderState::moreSampleToRead() const
{
  bool rv = mClusterSize > 0;
#ifdef ULDEBUG
  debugHeader() << fmt::format("moreSampleToRead {} clustersize {} nof10BitWords {}\n", rv, mClusterSize, mNof10BitWords);
#endif
  return rv;
}

bool DecoderState::headerIsComplete() const
{
  bool rv = mHeaderParts.size() == 5;
#ifdef ULDEBUG
  debugHeader() << fmt::format("headerIsComplete {}\n", rv);
#endif
  return rv;
}

bool DecoderState::moreWordsToRead() const
{
  bool rv = mNof10BitWords > 0;
#ifdef ULDEBUG
  debugHeader() << fmt::format("moreWordsToRead {} n10 {}\n", rv, mNof10BitWords);
#endif
  return rv;
}

std::ostream& operator<<(std::ostream& os, const DecoderState& ds)
{
  os << fmt::format("DecoderState {} data=0X{:08X} n10={:4d} csize={:4d} ctime={:4d} maskIndex={:4d} ",
                    asString(ds.dsId()), ds.mData,
                    ds.mNof10BitWords, ds.mClusterSize, ds.mClusterTime, ds.mMaskIndex);
  os << fmt::format("header({})= ", ds.mHeaderParts.size());
  for (auto h : ds.mHeaderParts) {
    os << fmt::format("{:4d} ", h);
  }
  os << fmt::format("samples({})= ", ds.mSamples.size());
  for (auto s : ds.mSamples) {
    os << fmt::format("{:4d} ", s);
  }

  if (ds.hasError()) {
    os << " ERROR:" << ds.errorMessage();
  }
  return os;
}

} // namespace o2::mch::raw
