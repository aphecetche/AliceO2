// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHRaw/GBTDecoder.h"
#include <fmt/printf.h>
#include "MakeArray.h"
using namespace o2::mch::raw;
using namespace boost::multiprecision;
// FIXME: instead of i % 16 for elinkid , use 0..39 and let the elinkencoder compute the dsid within
// the right range 0..15 itself ? Or have the mapping at this level already ?
GBTDecoder::GBTDecoder(int linkId, SampaChannelHandler sampaChannelHandler) : mId(linkId), mElinks{::makeArray<40>([sampaChannelHandler](size_t i) { return ElinkDecoder(i % 16, sampaChannelHandler); })}, mNofGBTWordsSeens{0}
{
  if (linkId < 0 || linkId > 23) {
    throw std::invalid_argument(fmt::sprintf("linkId %d should be between 0 and 23", linkId));
  }
}

void GBTDecoder::append(uint128_t w)
{
  ++mNofGBTWordsSeens;
  // dispatch the 80 bits to the underlaying elinks (2 bits per elink)
  for (int i = 0; i < 80; i += 2) {
    mElinks[i / 2].append(bit_test(w, i), bit_test(w, i + 1));
  }
}

void GBTDecoder::printStatus()
{
  std::cout << fmt::format("GBTDecoder({}) # GBT words seen {}\n", mId, mNofGBTWordsSeens);
  for (auto& e : mElinks) {
    std::cout << e << "\n";
  }
}
