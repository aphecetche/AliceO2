// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_ENCODER_DUAL_SAMPA_ELECTRONIC_LOCATION_H
#define O2_MCH_RAW_ENCODER_DUAL_SAMPA_ELECTRONIC_LOCATION_H

#include <cstdint>

namespace o2::mch::raw
{
class DualSampaElectronicLocation
{
 public:
  explicit DualSampaElectronicLocation(uint16_t solarId, uint8_t elinkGroupId, uint8_t elinkIndex);

  constexpr uint8_t elinkGroupId()
  {
    return mElinkGroupId;
  }
  constexpr uint8_t elinkId()
  {
    return mElinkGroupId * 5 + mElinkIndexInGroup;
  }

  static DualSampaElectronicLocation Invalid()
  {
    return DualSampaElectronicLocation{0, 8, 5};
  }

  constexpr uint16_t solarId()
  {
    return mSolarId;
  }

 private:
  uint16_t mSolarId;
  uint8_t mElinkGroupId;      // 0..7
  uint8_t mElinkIndexInGroup; // 0..4
};
} // namespace o2::mch::raw

#endif
