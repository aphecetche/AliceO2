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
#include <gsl/span>
#include "MCHRaw/SampaCluster.h"

namespace o2
{
namespace mch
{
namespace raw
{

class ElinkEncoder
{
 public:
  explicit ElinkEncoder(uint8_t id, uint8_t chip, int phase = 0,
                        bool chargeSumMode = true);

  void addChannelData(uint8_t chId, const std::vector<SampaCluster>& data);

  void clear();

  void fillWithSync(int upto);

  bool get(int i) const;

  uint8_t id() const;

  int len() const;

  void resetLocalBunchCrossing();

  friend std::ostream& operator<<(std::ostream& os, const ElinkEncoder& enc);

  uint64_t range(int a, int b) const;

 private:
  void append(bool value);
  void append(const SampaCluster& sc);
  void append10(uint16_t value);
  void append20(uint32_t value);
  void append50(uint64_t value);
  void assertNotMixingClusters(const std::vector<SampaCluster>& data) const;
  void assertPhase();
  void assertSync();
  uint64_t nofSync() const { return mNofSync; }
  void setHeader(uint8_t chId, uint16_t n10);

 private:
  uint8_t mId;
  uint8_t mChipAddress;
  SampaHeader mSampaHeader;
  BitSet mBitSet;
  uint64_t mNofSync;
  int mSyncIndex;
  uint64_t mNofBitSeen;
  int mPhase;
  uint32_t mLocalBunchCrossing;
  bool mChargeSumMode;
};

} // namespace raw
} // namespace mch
} // namespace o2
#endif
