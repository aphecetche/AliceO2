// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_DECODER_H
#define O2_MCH_RAW_DECODER_H

#include <cstdlib>
#include <gsl/span>
#include "MCHRaw/SampaChannelHandler.h"
#include "MCHRaw/RawDataHeaderHandler.h"

namespace o2
{
namespace mch
{
/// Classes and functions to deal with MCH Raw Data Formats.
namespace raw
{

using Decoder = std::function<void(gsl::span<uint8_t> buffer)>;

Decoder createBareDecoder(RawDataHeaderHandler rdhHandler, SampaChannelHandler channelHandler, bool chargeSumMode = true);

Decoder createUserLogicDecoder(RawDataHeaderHandler rdhHandler, SampaChannelHandler channelHandler);

} // namespace raw
} // namespace mch
} // namespace o2
#endif
