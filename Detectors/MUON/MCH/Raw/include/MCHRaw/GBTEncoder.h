// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_GBT_ENCODER_H
#define O2_MCH_RAW_GBT_ENCODER_H

#include <array>
#include "MCHRaw/ElinkEncoder.h"
#include "MCHRaw/GBTWord.h"

namespace o2
{
namespace mch
{
namespace raw
{

class GBTEncoder
{
 public:
  GBTEncoder(int linkId);

  void addChannelChargeSum(uint32_t bx, uint8_t elinkId, uint8_t chId, uint16_t timestamp, uint32_t chargeSum);

  void finalize();

  size_t size() const;
  GBTWord getWord(int i) const;

  void clear();

  void printStatus();

 private:
  void elink2gbt();

 private:
  int mId;
  bool mElinksInSync;
  std::array<ElinkEncoder, 40> mElinks;
  std::vector<GBTWord> mGBTWords;
};
} // namespace raw
} // namespace mch
} // namespace o2
#endif
