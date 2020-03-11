// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

namespace o2::mch::raw::ul
{

template <typename CHARGESUM>
Decoder_<CHARGESUM>::Decoder_(DsElecId dsId, SampaChannelHandler sampaChannelHandler)
  : mDsId{dsId},
    mSampaChannelHandler{sampaChannelHandler}
{
}

template <typename CHARGESUM>
void Decoder_<CHARGESUM>::decrementClusterSize()
{
  --mClusterSize;
}

template <typename CHARGESUM>
std::string Decoder_<CHARGESUM>::setClusterSize(uint16_t value)
{
  mClusterSize = value;
  int checkSize = mClusterSize + 2 - mSampaHeader.nof10BitWords();
  std::string checkSizeMsg("");
#ifdef ULDEBUG
  if (checkSize < 0) {
    checkSizeMsg = "cluster size smaller than nof10BitWords";
  } else if (checkSize > 0) {
    checkSizeMsg = "cluster size bigger than nof10BitWords !!!";
  }
  debugHeader(*this) << " -> size=" << mClusterSize << " maskIndex=" << mMaskIndex
                     << " nof10BitWords=" << mSampaHeader.nof10BitWords()
                     << " " << checkSizeMsg << "\n";
#endif
  if (checkSize) {
    if (checkSize < 0) {
      checkSizeMsg = "cluster size smaller than nof10BitWords";
    } else if (checkSize > 0) {
      checkSizeMsg = "cluster size bigger than nof10BitWords !!!";
    }
    std::cout << "RETURNING " << checkSizeMsg << "\n";
  }
  return checkSizeMsg;
}

template <typename CHARGESUM>
void Decoder_<CHARGESUM>::setClusterTime(uint16_t value)
{
  mClusterTime = value;
#ifdef ULDEBUG
  debugHeader(*this) << " -> time=" << mClusterTime << " maskIndex=" << mMaskIndex << "\n";
#endif
}

template <typename CHARGESUM>
void Decoder_<CHARGESUM>::reset()
{
  mMaskIndex = mMasks.size();
  mHeaderParts.clear();
  mClusterSize = 0;
  mNof10BitWords = 0;
}

template <typename CHARGESUM>
void Decoder_<CHARGESUM>::setData(uint64_t data)
{
  mData = data;
  mMaskIndex = 0;
#ifdef ULDEBUG
  debugHeader(*this) << fmt::format(">>>>> setData {:08X} maskIndex {} 10bits=", mData, mMaskIndex);
  for (int i = 0; i < mMasks.size(); i++) {
    std::cout << fmt::format("{:2d} ", data10(mData, i));
  }
  std::cout << "\n";
#endif
}

template <typename CHARGESUM>
uint16_t Decoder_<CHARGESUM>::data10(uint64_t value, size_t index) const
{
  if (index < 0 || index >= mMasks.size()) {
    std::cout << "-- ULDEBUG -- " << fmt::format("index {} is out of range\n", index);
    return 0;
  }
  uint64_t m = mMasks[index];
  return static_cast<uint16_t>((value & m) >> (index * 10) & 0x3FF);
}
template <typename CHARGESUM>
uint16_t Decoder_<CHARGESUM>::pop10()
{
  auto rv = data10(mData, mMaskIndex);
  mNof10BitWords = std::max(0, mNof10BitWords - 1);
  mMaskIndex = std::min(mMasks.size(), mMaskIndex + 1);
  return rv;
}
template <typename CHARGESUM>
void Decoder_<CHARGESUM>::addSample(uint16_t sample)
{
#ifdef ULDEBUG
  debugHeader(*this) << "sample = " << sample << "\n";
#endif
  mSamples.emplace_back(sample);

  if (mClusterSize == 0) {
    // a mCluster is ready, send it
#ifdef ULDEBUG
    std::stringstream s;
    s << SampaCluster(mClusterTime, mSamples);
    debugHeader(*this) << fmt::format(" calling channelHandler for {} ch {} = {}\n",
                                      asString(mDsId), channelNumber64(mSampaHeader), s.str());
#endif
    mSampaChannelHandler(mDsId, channelNumber64(mSampaHeader), SampaCluster(mClusterTime, mSamples));
    mSamples.clear();
  }
}

template <typename CHARGESUM>
void Decoder_<CHARGESUM>::completeHeader()
{
  uint64_t header{0};
  for (auto i = 0; i < mHeaderParts.size(); i++) {
    header += (static_cast<uint64_t>(mHeaderParts[i]) << (10 * i));
  }

  mSampaHeader = SampaHeader(header);
  mNof10BitWords = mSampaHeader.nof10BitWords();

#ifdef ULDEBUG
  debugHeader(*this)
    << fmt::format(">>>>> completeHeader {:013X}\n", header)
    << "\n"
    << mSampaHeader << "\n";
#endif

  mHeaderParts.clear();
}

template <typename CHARGESUM>
void Decoder_<CHARGESUM>::addHeaderPart(uint16_t a)
{
  mHeaderParts.emplace_back(a);
#ifdef ULDEBUG
  debugHeader(*this)
    << fmt::format(">>>>> readHeader {:08X}", a);
  for (auto h : mHeaderParts) {
    std::cout << fmt::format("{:4d} ", h);
  }
  std::cout << "\n";
#endif
}

template <typename CHARGESUM>
void Decoder_<CHARGESUM>::addChargeSum(uint16_t b, uint16_t a)
{
  // a mCluster is ready, send it
  uint32_t q = (((static_cast<uint32_t>(a) & 0x3FF) << 10) | (static_cast<uint32_t>(b) & 0x3FF));
#ifdef ULDEBUG
  debugHeader(*this)
    << "chargeSum = " << q << "\n";
#endif
  mSampaChannelHandler(mDsId,
                       channelNumber64(mSampaHeader),
                       SampaCluster(mClusterTime, q));
}

template <typename CHARGESUM>
bool Decoder_<CHARGESUM>::moreDataAvailable() const
{
  bool rv = mMaskIndex < mMasks.size();
#ifdef ULDEBUG
  debugHeader(*this) << fmt::format("moreDataAvailable {} maskIndex {}\n", rv, mMaskIndex);
#endif
  return rv;
}

template <typename CHARGESUM>
bool Decoder_<CHARGESUM>::moreSampleToRead() const
{
  bool rv = mClusterSize > 0;
#ifdef ULDEBUG
  debugHeader(*this) << fmt::format("moreSampleToRead {} clustersize {} nof10BitWords {}\n", rv, mClusterSize, mNof10BitWords);
#endif
  return rv;
}

template <typename CHARGESUM>
bool Decoder_<CHARGESUM>::headerIsComplete() const
{
  bool rv = mHeaderParts.size() == 5;
#ifdef ULDEBUG
  debugHeader(*this) << fmt::format("headerIsComplete {}\n", rv);
#endif
  return rv;
}

template <typename CHARGESUM>
bool Decoder_<CHARGESUM>::moreWordsToRead() const
{
  bool rv = mNof10BitWords > 0;
#ifdef ULDEBUG
  debugHeader(*this) << fmt::format("moreWordsToRead {} n10 {}\n", rv, mNof10BitWords);
#endif
  return rv;
}

#ifdef ULDEBUG
template <typename CHARGESUM>
DsElecId Decoder_<CHARGESUM>::dsId() const
{
  return mDsId;
}
#endif

} // namespace o2::mch::raw::ul
