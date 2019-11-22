// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "DecoderImpl.h"
#include "CRUDecoder.h"
#include "BareGBTDecoder.h"
#include "UserLogicGBTDecoder.h"
#include "Headers/RAWDataHeader.h"
#include "MCHRawCommon/DataFormats.h"

namespace o2::mch::raw::impl
{

template <typename FORMAT, typename CHARGESUM, typename RDH>
Decoder createDecoder(RawDataHeaderHandler<RDH> rdhHandler,
                      SampaChannelHandler channelHandler);

// as functions cannot be partially specialized, we create a struct
// (as struct _can_ be specialized) and use it in our function(s)
template <typename FORMAT, typename CHARGESUM, typename RDH>
struct DecoderCreator {
  static Decoder _(RawDataHeaderHandler<RDH> rdhHandler,
                   SampaChannelHandler channelHandler);
};

template <typename CHARGESUM, typename RDH>
struct DecoderCreator<BareFormat, CHARGESUM, RDH> {
  static Decoder _(RawDataHeaderHandler<RDH> rdhHandler,
                   SampaChannelHandler channelHandler)
  {
    return DecoderImpl<CHARGESUM, RDH, CRUDecoder<BareGBTDecoder<CHARGESUM>, CHARGESUM>>(rdhHandler, channelHandler);
  }
};

template <typename CHARGESUM, typename RDH>
struct DecoderCreator<UserLogicFormat, CHARGESUM, RDH> {
  static Decoder _(RawDataHeaderHandler<RDH> rdhHandler,
                   SampaChannelHandler channelHandler)
  {
    return DecoderImpl<CHARGESUM, RDH, CRUDecoder<UserLogicGBTDecoder<CHARGESUM>, CHARGESUM>>(rdhHandler, channelHandler);
  }
};
} // namespace o2::mch::raw::impl

namespace o2::mch::raw
{

template <typename FORMAT, typename CHARGESUM, typename RDH>
Decoder createDecoder(RawDataHeaderHandler<RDH> rdhHandler, SampaChannelHandler channelHandler)
{
  return impl::DecoderCreator<FORMAT, CHARGESUM, RDH>::_(rdhHandler, channelHandler);
}

// define only the specialization we use

using RDHv4 = o2::header::RAWDataHeaderV4;

template Decoder createDecoder<BareFormat, SampleMode, RDHv4>(RawDataHeaderHandler<RDHv4>, SampaChannelHandler);
template Decoder createDecoder<BareFormat, ChargeSumMode, RDHv4>(RawDataHeaderHandler<RDHv4>, SampaChannelHandler);
template Decoder createDecoder<UserLogicFormat, SampleMode, RDHv4>(RawDataHeaderHandler<RDHv4>, SampaChannelHandler);
template Decoder createDecoder<UserLogicFormat, ChargeSumMode, RDHv4>(RawDataHeaderHandler<RDHv4>, SampaChannelHandler);
} // namespace o2::mch::raw
