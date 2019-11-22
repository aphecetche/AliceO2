// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHRawEncoder/DualSampaElectronicLocation.h"
#include "Assertions.h"

namespace o2::mch::raw
{
DualSampaElectronicLocation::DualSampaElectronicLocation(uint16_t solarId, uint8_t elinkGroupId, uint8_t elinkIndex)
  : mSolarId{solarId}, mElinkGroupId{elinkGroupId}, mElinkIndexInGroup{elinkIndex}
{
  impl::assertIsInRange<uint8_t>("elinkGroupId", mElinkGroupId, 0, 7);
  impl::assertIsInRange<uint8_t>("elinkIndex", mElinkIndexInGroup, 0, 4);
}

} // namespace o2::mch::raw
