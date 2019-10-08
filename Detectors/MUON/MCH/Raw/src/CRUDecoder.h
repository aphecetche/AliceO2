// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_CRU_DECODER_H
#define O2_MCH_RAW_CRU_DECODER_H

#include "MCHRaw/SampaChannelHandler.h"
#include <gsl/span>
#include "MCHRaw/GBTDecoder.h"
#include <array>

namespace o2
{
namespace mch
{
namespace raw
{
class CRUDecoder
{
 public:
  explicit CRUDecoder(int cruId, SampaChannelHandler channelHandler);

  void decode(int gbtid, gsl::span<uint32_t> buffer);

 private:
  int mCruId;
  std::array<GBTDecoder, 24> mGbtDecoders;
};

} // namespace raw
} // namespace mch
} // namespace o2
#endif
