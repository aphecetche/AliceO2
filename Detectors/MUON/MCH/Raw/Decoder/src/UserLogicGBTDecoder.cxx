// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "UserLogicGBTDecoder.h"
#include <fmt/printf.h>
#include <fmt/format.h>
#include "MakeArray.h"
#include "Assertions.h"
#include <iostream>
#include <boost/multiprecision/cpp_int.hpp>

using namespace o2::mch::raw;
using namespace boost::multiprecision;

UserLogicGBTDecoder::UserLogicGBTDecoder(int cruId,
                                         int gbtId,
                                         SampaChannelHandler sampaChannelHandler,
                                         bool chargeSumMode)
  : mCruId(cruId),
    mGbtId(gbtId),
    mDSDecoders{impl::makeArray<40>([=](size_t i) { return UserLogicDSDecoder(cruId, i, sampaChannelHandler, chargeSumMode); })},
    mNofGbtWordsSeens{0}
{
  impl::assertIsInRange("gbtId", gbtId, 0, 23);
}

void UserLogicGBTDecoder::append(gsl::span<uint8_t> buffer)
{
  if (buffer.size() % 8) {
    throw std::invalid_argument("buffer size should be a multiple of 8");
  }
  for (size_t i = 0; i < buffer.size(); i += 8) {

    uint64_t word = (static_cast<uint64_t>(buffer[i + 0])) |
                    (static_cast<uint64_t>(buffer[i + 1]) << 8) |
                    (static_cast<uint64_t>(buffer[i + 2]) << 16) |
                    (static_cast<uint64_t>(buffer[i + 3]) << 24) |
                    (static_cast<uint64_t>(buffer[i + 4]) << 32) |
                    (static_cast<uint64_t>(buffer[i + 5]) << 40) |
                    (static_cast<uint64_t>(buffer[i + 6]) << 48) |
                    (static_cast<uint64_t>(buffer[i + 7]) << 56);

    if (word == 0) {
      continue;
    }
    if (word == 0xFEEDDEEDFEEDDEED) {
      continue;
    }
    int gbt = (word >> 59) & 0x1F;
    if (gbt != mGbtId) {
      std::cout << fmt::format("warning : gbt {} != expected {} word={:08X}\n", gbt, mGbtId, word);
      // throw std::invalid_argument(fmt::format("gbt {} != expected {} word={:X}\n", gbt, mGbtId, word));
    }

    uint16_t dsid = (word >> 53) & 0x3F;
    impl::assertIsInRange("dsid", dsid, 0, 39);

    // the remaining 50 bits are passed to the DSDecoder
    uint64_t data = word & UINT64_C(0x003FFFFFFFFFFFFF);
    mDSDecoders[dsid].append(data);
  }
}

void UserLogicGBTDecoder::reset()
{
}
