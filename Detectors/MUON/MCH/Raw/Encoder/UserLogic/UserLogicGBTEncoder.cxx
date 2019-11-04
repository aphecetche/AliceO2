// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "UserLogicGBTEncoder.h"

namespace o2::mch::raw
{

UserLogicGBTEncoder::UserLogicGBTEncoder(int cruId, int gbtId, bool chargeSumMode)
{
}
int UserLogicGBTEncoder::id() const
{
  return 0;
}

void UserLogicGBTEncoder::addChannelData(uint8_t elinkId, uint8_t chId, const std::vector<SampaCluster>& data)
{
}

size_t UserLogicGBTEncoder::moveToBuffer(std::vector<uint8_t>& buffer)
{
  return 0;
}

void UserLogicGBTEncoder::printStatus(int maxelink) const
{
}
} // namespace o2::mch::raw
