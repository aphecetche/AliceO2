// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "CRUDecoder.h"
#include "MakeArray.h"
#include "Assertions.h"
#include <iostream>
#include <fmt/format.h>

namespace o2
{
namespace mch
{
namespace raw
{

CRUDecoder::CRUDecoder(int cruId, SampaChannelHandler sampaChannelHandler) : mCruId{cruId}, mGbtDecoders{::makeArray<24>([=](size_t i) { return GBTDecoder(cruId, i, sampaChannelHandler); })}
{
}

void CRUDecoder::decode(int gbtId, gsl::span<uint32_t> buffer)
{
  assertIsInRange("gbtId", gbtId, 0, mGbtDecoders.size());
  if (buffer.size() % 4) {
    throw std::invalid_argument("buffer size should be a multiple of 4");
  }
  auto& gbt = mGbtDecoders[gbtId];
  for (auto i = 0; i < buffer.size(); i += 4) {
    gbt.append(buffer[i], buffer[i + 1], buffer[i + 2], buffer[i + 3]);
  }
}

} // namespace raw
} // namespace mch
} // namespace o2
