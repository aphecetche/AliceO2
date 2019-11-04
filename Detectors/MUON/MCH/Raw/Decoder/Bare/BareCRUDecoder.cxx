// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "BareCRUDecoder.h"
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

BareCRUDecoder::BareCRUDecoder(int cruId,
                               SampaChannelHandler sampaChannelHandler,
                               bool chargeSumMode)
  : mCruId{cruId},
    mGbtDecoders{::makeArray<24>([=](size_t i) { return BareGBTDecoder(cruId, i, sampaChannelHandler, chargeSumMode); })}
{
}

void BareCRUDecoder::decode(int gbtId, gsl::span<uint8_t> buffer)
{
  assertIsInRange("gbtId", gbtId, 0, mGbtDecoders.size());
  if (buffer.size() % 16) {
    throw std::invalid_argument("buffer size should be a multiple of 16");
  }
  mGbtDecoders[gbtId].append(buffer);
}

void BareCRUDecoder::reset()
{
  for (auto& g : mGbtDecoders) {
    g.reset();
  }
}
} // namespace raw
} // namespace mch
} // namespace o2
