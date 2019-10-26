// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHRaw/Decoder.h"
#include "BareDecoder.h"
#include "UserLogicDecoder.h"

namespace o2
{
namespace mch
{
namespace raw
{

Decoder createBareDecoder(RawDataHeaderHandler rdhHandler,
                          SampaChannelHandler channelHandler,
                          bool chargeSumMode)

{
  return BareDecoder(rdhHandler, channelHandler, chargeSumMode);
}

Decoder createUserLogicDecoder(RawDataHeaderHandler rdhHandler,
                               SampaChannelHandler channelHandler)

{
  return UserLogicDecoder(rdhHandler, channelHandler);
}

} // namespace raw
} // namespace mch
} // namespace o2
