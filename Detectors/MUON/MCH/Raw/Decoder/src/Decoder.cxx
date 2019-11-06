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

namespace o2::mch::raw
{

template <>
Decoder createBareDecoder(RawDataHeaderHandler<o2::header::RAWDataHeaderV4> rdhHandler, SampaChannelHandler channelHandler, bool chargeSumMode)
{
  return DecoderImpl<o2::header::RAWDataHeaderV4, CRUDecoder<BareGBTDecoder>>(rdhHandler, channelHandler, chargeSumMode);
}

template <>
Decoder createUserLogicDecoder(RawDataHeaderHandler<o2::header::RAWDataHeaderV4> rdhHandler, SampaChannelHandler channelHandler, bool chargeSumMode)
{
  return DecoderImpl<o2::header::RAWDataHeaderV4, CRUDecoder<UserLogicGBTDecoder>>(rdhHandler, channelHandler, chargeSumMode);
}

} // namespace o2::mch::raw
