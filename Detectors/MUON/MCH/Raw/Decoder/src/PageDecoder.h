// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_DECODER_PAGE_DECODER_H
#define O2_MCH_RAW_DECODER_PAGE_DECODER_H

#include <functional>
#include <gsl/span>
#include <map>
#include "MCHRawDecoder/Decoder.h"
#include "MCHRawElecMap/CruLinkId.h"

//
// A PageDecoder dispatches its input page (a page = RDH+payload)
// to an actual payload decoder depending on the information
// in the RDH part of the page
//

namespace o2::mch::raw
{

template <typename FORMAT>
uint16_t cru2id(uint32_t cruLinkId)
{
  return 0;
}

template <typename RDH, typename FORMAT, typename PAYLOADDECODER>
class PageDecoder
{
 public:
  PageDecoder(SampaChannelHandler sampaChannelHandler) : mSampaChannelHandler{sampaChannelHandler} {}

  void operator()(gsl::span<std::uint8_t> pageBuffer)
  {
    auto rdh = createRDH<RDH>(pageBuffer);
    int cruId = rdh.feeId & 0xFF;
    int linkId = rdh.linkID;
    uint32_t orbit = rdhOrbit(rdh);

    uint32_t cruLinkId = o2::mch::raw::encode(CruLinkId(cruId, linkId));

    auto p = mPayloadDecoders.find(cruLinkId);

    if (p == mPayloadDecoders.end()) {
      auto decoderID = cru2id<FORMAT>(cruLinkId);
      mPayloadDecoders.emplace(cruLinkId, PAYLOADDECODER(decoderID, mSampaChannelHandler));
      p = mPayloadDecoders.find(cruLinkId);
    }

    p->second.process(orbit, pageBuffer.subspan(sizeof(rdh), rdhPayloadSize(rdh)));
  }

 private:
  SampaChannelHandler mSampaChannelHandler;
  std::map<uint32_t, PAYLOADDECODER> mPayloadDecoders;
};

} // namespace o2::mch::raw

#endif
