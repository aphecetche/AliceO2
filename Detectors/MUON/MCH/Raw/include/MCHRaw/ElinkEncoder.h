// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_ELINK_ENCODER_H
#define O2_MCH_RAW_ELINK_ENCODER_H

#include "BitSet.h"
#include "SampaHeader.h"
#include <vector>
#include <iostream>

namespace o2
{
namespace mch
{
namespace raw
{

class ElinkEncoder
{
 public:
  explicit ElinkEncoder(uint8_t id, uint8_t dsid);

  uint8_t id() const;

  void bunchCrossingCounter(uint32_t bx);
  void addChannelSamples(uint8_t chId, uint16_t timestamp, const std::vector<uint16_t>& samples);
  void addChannelChargeSum(uint8_t chId, uint16_t timestamp, uint32_t chargeSum,
                           uint16_t nsamples = 1);
  void addSync();
  void addRandomBits(int n);
  int fillWithSync(int upto);

  bool get(int i) const;
  int len() const;
  void clear();

  uint64_t range(int a, int b) const;

  friend std::ostream& operator<<(std::ostream& os, const ElinkEncoder& enc);

 private:
  void addHeader(uint8_t chId, const std::vector<uint16_t>& samples);
  void addHeader(uint8_t chId, uint32_t chargeSum);
  void setHeader(uint8_t chId, uint16_t n10);

 private:
  uint8_t mId;
  uint8_t mDsId;
  SampaHeader mSampaHeader;
  BitSet mBitSet;
};

} // namespace raw
} // namespace mch
} // namespace o2
#endif
