// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_GBT_DECODER_H
#define O2_MCH_RAW_GBT_DECODER_H

#include <array>
#include "MCHRaw/ElinkDecoder.h"
#include "MCHRaw/SampaChannelHandler.h"
#include "MCHRaw/GBT.h"

namespace o2
{
namespace mch
{
namespace raw
{

class GBTDecoder
{
 public:
  GBTDecoder(int linkId, SampaChannelHandler sampaChannelHandler);

  void append(uint128_t w);

  void printStatus(int maxelink = -1) const;

  void finalize();

 private:
  int mId;
  std::array<ElinkDecoder, 40> mElinks;
  int mNofGBTWordsSeens;
};
} // namespace raw
} // namespace mch
} // namespace o2
#endif
