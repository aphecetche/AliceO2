// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_ENCODER_USER_LOGIC_GBT_ENCODER_H
#define O2_MCH_RAW_ENCODER_USER_LOGIC_GBT_ENCODER_H

#include <cstdlib>
#include "MCHRawCommon/SampaCluster.h"
#include "UserLogicElinkEncoder.h"
#include <vector>
#include <array>

namespace o2::mch::raw
{

class UserLogicGBTEncoder
{
 public:
  UserLogicGBTEncoder(int cruId, int gbtId, bool chargeSumMode = true);

  int id() const { return mGbtId; }

  void addChannelData(uint8_t elinkId, uint8_t chId, const std::vector<SampaCluster>& data);

  size_t moveToBuffer(std::vector<uint8_t>& buffer);

  void printStatus(int maxelink = -1) const;

  static bool forceNoPhase;

 private:
  int mCruId;
  int mGbtId;
  uint64_t mGbtIdMask;
  std::array<UserLogicElinkEncoder, 40> mElinks;
};

} // namespace o2::mch::raw

#endif
