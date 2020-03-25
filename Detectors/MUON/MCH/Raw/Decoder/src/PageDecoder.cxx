// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "BareGBTDecoder.h"
#include "Headers/RAWDataHeader.h"
#include "MCHRawCommon/DataFormats.h"
#include "MCHRawCommon/RDHManip.h"
#include "MCHRawDecoder/PageDecoder.h"
#include "UserLogicEndpointDecoder.h"
#include <iostream>

namespace o2::mch::raw
{

namespace impl
{
template <typename FORMAT>
uint16_t cru2id(uint32_t cruLinkId)
{
  return 0;
}

uint16_t CRUID_MASK = 0xFF;
uint16_t CHARGESUM_MASK = 0x100;

template <typename RDH, typename FORMAT, typename PAYLOADDECODER>
class PageDecoderImpl
{
 private:
 public:
  PageDecoderImpl(SampaChannelHandler sampaChannelHandler) : mSampaChannelHandler{sampaChannelHandler}
  {
    std::cout << __PRETTY_FUNCTION__ << "\n";
  }

  void operator()(Page page)
  {
    auto rdh = createRDH<RDH>(page);
    int cruId = rdh.feeId & CRUID_MASK;
    int linkId = rdh.linkID;
    uint32_t orbit = rdhOrbit(rdh);

    uint32_t cruLinkId = rdh.feeId; // FIXME o2::mch::raw::encode(CruLinkId(cruId, linkId));

    auto p = mPayloadDecoders.find(cruLinkId);

    if (p == mPayloadDecoders.end()) {
      auto decoderID = cru2id<FORMAT>(cruLinkId);
      mPayloadDecoders.emplace(cruLinkId, PAYLOADDECODER(decoderID, mSampaChannelHandler));
      p = mPayloadDecoders.find(cruLinkId);
    }

    p->second.process(orbit, page.subspan(sizeof(rdh), rdhPayloadSize(rdh)));
  }

 private:
  SampaChannelHandler mSampaChannelHandler;
  std::map<uint32_t, PAYLOADDECODER> mPayloadDecoders;
};

template <typename RDH>
void print(const RDH& rdh);

template <typename RDH>
class PageParser
{
 public:
  void operator()(RawBuffer buffer, PageDecoder pageDecoder)
  {
    size_t pos{0};
    while (pos < buffer.size_bytes() - sizeof(RDH)) {
      auto rdh = createRDH<RDH>(buffer.subspan(pos, sizeof(RDH)));
      print(rdh);
      auto payloadSize = rdhPayloadSize(rdh);
      pageDecoder(buffer.subspan(pos, sizeof(RDH) + payloadSize));
      pos += sizeof(RDH) + payloadSize;
    }
  }
};

} // namespace impl
using V4 = o2::header::RAWDataHeaderV4;
//using V5 = o2::header::RAWDataHeaderV5;

template <>
void impl::print(const V4& rdh)
{
  std::cout << rdhOrbit(rdh) << " " << rdhBunchCrossing(rdh) << " " << rdhFeeId(rdh) << "\n";
}

PageDecoder createPageDecoder(RawBuffer rdhBuffer, SampaChannelHandler channelHandler)
{
  auto rdh = createRDH<V4>(rdhBuffer);
  if (isValid(rdh)) {
    if (rdhLinkId(rdh) == 15) {
      if (rdhFeeId(rdh) & impl::CHARGESUM_MASK) {
        return impl::PageDecoderImpl<V4, UserLogicFormat, UserLogicEndpointDecoder<ChargeSumMode>>(channelHandler);
      } else {
        return impl::PageDecoderImpl<V4, UserLogicFormat, UserLogicEndpointDecoder<SampleMode>>(channelHandler);
      }
    } else {
      if (rdhFeeId(rdh) & impl::CHARGESUM_MASK) {
        return impl::PageDecoderImpl<V4, BareFormat, BareGBTDecoder<SampleMode>>(channelHandler);
      } else {
        return impl::PageDecoderImpl<V4, BareFormat, BareGBTDecoder<ChargeSumMode>>(channelHandler);
      }
    }
  }
  throw std::invalid_argument("do not know how to create a page decoder for this RDH type\n");
}

PageParser createPageParser(RawBuffer buffer)
{
  auto rdh = createRDH<V4>(buffer);
  if (isValid(rdh)) {
    return impl::PageParser<V4>();
  }
  throw std::invalid_argument("do not know how to create a page parser for this RDH type\n");
}

} // namespace o2::mch::raw
