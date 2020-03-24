// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHRawDecoder/Decoder.h"
#include "Headers/RAWDataHeader.h"
#include "MCHRawCommon/RDHManip.h"
#include "MCHRawCommon/DataFormats.h"
#include "PageDecoder.h"
#include "UserLogicEndpointDecoder.h"
#include "BareGBTDecoder.h"

namespace o2::mch::raw
{

using V4 = o2::header::RAWDataHeaderV4;
//using V5 = o2::header::RAWDataHeaderV5;

Decoder createDecoder(gsl::span<std::uint8_t> rdhBuffer, SampaChannelHandler channelHandler)
{
  auto rdh = createRDH<V4>(rdhBuffer);
  if (isValid(rdh)) {
    if (rdhLinkId(rdh) == 15) {
      return PageDecoder<V4, UserLogicFormat, UserLogicEndpointDecoder<ChargeSumMode>>(channelHandler);
    } else {
      //FIXME: get the data format from feeId
      return PageDecoder<V4, BareFormat, BareGBTDecoder<SampleMode>>(channelHandler);
    }
  }
  throw std::invalid_argument("do not know how to create a decoder for this RDH type\n");
}

} // namespace o2::mch::raw
