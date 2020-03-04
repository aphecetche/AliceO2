// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "Decoder.inl"
#include "UserLogicGBTDecoder.h"
#include "Headers/RAWDataHeader.h"
#include "MCHRawCommon/DataFormats.h"

namespace o2::mch::raw
{

template <typename CHARGESUM>
struct GBTDecoderTrait<UserLogicFormat, CHARGESUM> {
  using type = UserLogicGBTDecoder<CHARGESUM>;
};

// define only the specialization we use

using RDHv4 = o2::header::RAWDataHeaderV4;

template Decoder createDecoder<UserLogicFormat, SampleMode, RDHv4>(RawDataHeaderHandler<RDHv4>, SampaChannelHandler);
template Decoder createDecoder<UserLogicFormat, ChargeSumMode, RDHv4>(RawDataHeaderHandler<RDHv4>, SampaChannelHandler);
} // namespace o2::mch::raw
