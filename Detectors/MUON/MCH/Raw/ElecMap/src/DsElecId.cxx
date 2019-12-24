// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHRawElecMap/DsElecId.h"
#include "Assertions.h"
#include <fmt/format.h>
#include <iostream>

namespace o2::mch::raw
{
DsElecId::DsElecId(uint16_t solarId, uint8_t elinkGroupId, uint8_t elinkIndex)
  : mSolarId{solarId}, mElinkGroupId{elinkGroupId}, mElinkIndexInGroup{elinkIndex}
{
  impl::assertIsInRange("elinkGroupId", mElinkGroupId, 0, 7);
  impl::assertIsInRange("elinkIndex", mElinkIndexInGroup, 0, 4);
}

uint16_t encode(const DsElecId& id)
{
  return (id.solarId() & 0x3FF) | ((id.elinkGroupId() & 0x7) << 10) |
         ((id.elinkIndexInGroup() & 0x7) << 13);
}

DsElecId decodeDsElecId(uint16_t code)
{
  uint16_t solarId = code & 0x3FF;

  uint8_t groupId = (code & 0x1C00) >> 10;

  uint8_t index = (code & 0xE000) >> 13;

  return DsElecId(solarId, groupId, index);
}
std::ostream& operator<<(std::ostream& os, const DsElecId& id)
{
  std::cout << fmt::format("DsElecId(SOLAR=S{:4d} GROUP=J{:2d} INDEX=DS{:2d}) CODE={:8d}",
                           id.solarId(), id.elinkGroupId(), id.elinkIndexInGroup(), encode(id));
  return os;
}

std::string asString(DsElecId dsId)
{
  return fmt::format("S{}-J{}-DS{}", dsId.solarId(), dsId.elinkGroupId(), dsId.elinkIndexInGroup());
}

std::optional<uint8_t> groupFromElinkId(uint8_t elinkId)
{
  if (elinkId < 40) {
    return elinkId / 5;
  }
  return std::nullopt;
}

std::optional<uint8_t> indexFromElinkId(uint8_t elinkId)
{
  if (elinkId < 40) {
    return elinkId - (elinkId / 5) * 5;
  }
  return std::nullopt;
}
} // namespace o2::mch::raw
