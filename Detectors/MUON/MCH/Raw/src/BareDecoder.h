// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_BAREDECODER_H
#define O2_MCH_RAW_BAREDECODER_H

#include "CRUDecoder.h"
#include "MCHRaw/RawDataHeaderHandler.h"
#include "MCHRaw/SampaChannelHandler.h"
#include <cstdlib>
#include <gsl/span>
namespace o2
{
namespace mch
{
namespace raw
{
class BareDecoder
{
 public:
  BareDecoder(RawDataHeaderHandler rdhHandler, SampaChannelHandler channelHandler);

  int operator()(gsl::span<uint32_t> buffer);

 private:
  // FIXME: how many CRUs really ? 18 gives already 17280 elinks,
  // which is more than the number of dual sampas ?
  std::array<CRUDecoder, 18> mCruDecoders;
  RawDataHeaderHandler mRdhHandler;
};
} // namespace raw
} // namespace mch
} // namespace o2

#endif
