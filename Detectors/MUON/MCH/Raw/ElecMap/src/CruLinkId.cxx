// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHRawElecMap/CruLinkId.h"
#include "Assertions.h"
#include <fmt/format.h>
#include <iostream>

namespace o2::mch::raw
{

CruLinkId::CruLinkId(uint16_t cruId, uint8_t linkId)
  : mCruId(cruId), mLinkId(linkId)
{
  //impl::assertIsInRange("cruId", cruId, 0, xxx);
  impl::assertIsInRange("cruId", mLinkId, 0, 23);
}

uint32_t encode(const CruLinkId& id)
{
  return id.cruId() << 16 | id.linkId();
}

CruLinkId decodeCruLinkId(uint32_t x)
{
  uint16_t cruId = static_cast<uint16_t>((x & 0xFFFF0000) >> 16);
  uint16_t linkId = static_cast<uint8_t>(x & 0xFF);
  return CruLinkId(cruId, linkId);
}

std::ostream& operator<<(std::ostream& os, const CruLinkId& id)
{
  os << fmt::format("CruLinkId(CRU={:4d},LINK={:1d}) CODE={:8d}", id.cruId(), id.linkId(), encode(id));
  return os;
}

} // namespace o2::mch::raw
