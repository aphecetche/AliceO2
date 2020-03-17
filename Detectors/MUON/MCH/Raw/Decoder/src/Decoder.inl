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
struct GBTDecoderTrait;

namespace impl
{
template <typename FORMAT, typename CHARGESUM, typename RDH>
class DecoderImpl
{
  using GBTDecoder = typename GBTDecoderTrait<FORMAT, CHARGESUM>::type;
  using PAYLOADDECODER = PayloadDecoder<RDH, GBTDecoder>;

 public:
  DecoderImpl(RawDataHeaderHandler<RDH> rdhHandler, SampaChannelHandler channelHandler)
    : mPageParser(rdhHandler, PAYLOADDECODER(channelHandler))
  {
    static int i{0};
    channelHandler(DsElecId{0, 0, 0}, 0, SampaCluster{0, 0});
    std::cout << "CALLCRASH in pageParser i=" << i << "\n";
    i++;
  }

  DecoderStat operator()(gsl::span<uint8_t> buffer)
  {
    return mPageParser.parse(buffer);
  }

  PageParser<RDH, PAYLOADDECODER> mPageParser;
};
} // namespace impl

template <typename FORMAT, typename CHARGESUM, typename RDH>
Decoder createDecoder(RawDataHeaderHandler<RDH> rdhHandler, SampaChannelHandler channelHandler)
{
  return impl::DecoderImpl<FORMAT, CHARGESUM, RDH>(rdhHandler, channelHandler);
}

} // namespace o2::mch::raw

#endif
