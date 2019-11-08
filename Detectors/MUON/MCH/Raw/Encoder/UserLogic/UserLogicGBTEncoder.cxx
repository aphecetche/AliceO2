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
#include "Assertions.h"
#include "DumpBuffer.h"
#include "MakeArray.h"

namespace o2::mch::raw
{

bool UserLogicGBTEncoder::forceNoPhase{false};

UserLogicGBTEncoder::UserLogicGBTEncoder(int cruId, int gbtId, bool chargeSumMode)
  : mCruId(cruId),
    mGbtId(gbtId),
    mGbtIdMask((static_cast<uint64_t>(gbtId & 0x1F) << 59)),
    mElinks{::makeArray<40>([chargeSumMode](size_t i) { return UserLogicElinkEncoder(i, i % 16, 0, chargeSumMode); })}
{
}

void UserLogicGBTEncoder::addChannelData(uint8_t elinkId, uint8_t chId, const std::vector<SampaCluster>& data)
{
  assertIsInRange("elinkId", elinkId, 0, 39);
  mElinks[elinkId].addChannelData(chId, data);
}

size_t UserLogicGBTEncoder::moveToBuffer(std::vector<uint8_t>& buffer)
{
  std::cout << "UserLogicGBTEncoder::moveToBuffer : buffer=\n";
  // FIXME: here must add the GbtMask part to each word of the elinks buffers

  for (auto& elink : mElinks) {
    elink.moveToBuffer(buffer, mGbtIdMask);
  }

  dumpBuffer(gsl::span<uint64_t>(reinterpret_cast<uint64_t*>(&buffer[0]), buffer.size() / 8));
  return 0;
}

void UserLogicGBTEncoder::printStatus(int maxelink) const
{
}
} // namespace o2::mch::raw
