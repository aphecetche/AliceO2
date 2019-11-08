// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_USER_LOGIC_ELINK_ENCODER_H
#define O2_MCH_RAW_USER_LOGIC_ELINK_ENCODER_H

#include <cstdlib>
#include "MCHRawCommon/SampaCluster.h"
#include <vector>

namespace o2::mch::raw
{

class UserLogicElinkEncoder
{
 public:
  explicit UserLogicElinkEncoder(uint8_t elinkId, uint8_t chip, int phase = 0,
                                 bool chargeSumMode = true);

  void addChannelData(uint8_t chId, const std::vector<SampaCluster>& data);

  size_t moveToBuffer(std::vector<uint8_t>& buffer, uint64_t prefix);

 private:
  uint8_t mElinkId;     //< Elink id 0..39
  uint8_t mChipAddress; //< chip address 0..15
  bool mChargeSumMode;  //< whether or not we should encode 20 or 10 bits for data
  bool mHasSync;        //< whether or not we've already added a sync word
  std::vector<uint64_t> mBuffer;
};

} // namespace o2::mch::raw

#endif
