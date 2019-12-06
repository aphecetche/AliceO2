// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_ELECMAP_DS_ELEC_ID_H
#define O2_MCH_RAW_ELECMAP_DS_ELEC_ID_H

#include <cstdint>
#include <iosfwd>

namespace o2::mch::raw
{
class DsElecId
{
 public:
  explicit DsElecId(uint16_t solarId, uint8_t elinkGroupId, uint8_t elinkIndex);

  constexpr uint8_t elinkIndexInGroup() const
  {
    return mElinkIndexInGroup;
  }

  constexpr uint8_t elinkGroupId() const
  {
    return mElinkGroupId;
  }

  constexpr uint8_t elinkId() const
  {
    return mElinkGroupId * 5 + mElinkIndexInGroup;
  }

  constexpr uint16_t solarId() const
  {
    return mSolarId;
  }

  bool operator==(const DsElecId& rhs)
  {
    return mSolarId == rhs.mSolarId &&
           mElinkIndexInGroup == rhs.mElinkIndexInGroup &&
           mElinkGroupId == rhs.mElinkGroupId;
  }
  bool operator!=(const DsElecId& rhs)
  {
    return !(*this == rhs);
  }

 private:
  uint16_t mSolarId;
  uint8_t mElinkGroupId;      // 0..7
  uint8_t mElinkIndexInGroup; // 0..4
};

uint16_t encode(const DsElecId& id);

DsElecId decodeDsElecId(uint16_t code);

std::ostream& operator<<(std::ostream& os, const DsElecId& id);

} // namespace o2::mch::raw

#endif
