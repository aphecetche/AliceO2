// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_DECODER_INL
#define O2_MCH_RAW_DECODER_INL

#include "PageParser.h"
#include "MCHRawCommon/DataFormats.h"

namespace o2::mch::raw
{

template <typename FORMAT, typename CHARGESUM>
struct GBTDecoderTrait {
  using type = void;
};

template <typename FORMAT, typename CHARGESUM, typename RDH>
Decoder createDecoder(RawDataHeaderHandler<RDH> rdhHandler, SampaChannelHandler channelHandler)
{
  using GBTDecoder = typename GBTDecoderTrait<FORMAT, CHARGESUM>::type;
  using PAYLOADDECODER = PayloadDecoder<RDH, GBTDecoder>;
  return [rdhHandler, channelHandler](gsl::span<uint8_t> buffer) -> DecoderStat {
    /* static */ PageParser<RDH, PAYLOADDECODER> mPageParser(rdhHandler, PAYLOADDECODER(channelHandler));
    return mPageParser.parse(buffer);
  };
}

} // namespace o2::mch::raw

#endif
