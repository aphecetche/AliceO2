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
  if (linkId == 0) {
    // mElinks[0].verbose(true);
    // mElinks[2].verbose(true);
    // mElinks[4].verbose(true);
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

void GBTDecoder::append(uint32_t w0, uint32_t w1, uint32_t w2, uint32_t w3)
{
  uint128_t b0_31 = w0;
  uint128_t b31_65 = w1;
  uint128_t b66_95 = w2;
  uint128_t b96_127 = w3;

  uint128_t gbtWord = b0_31 & (b31_65 << 32) & (b66_95 << 64) & (b96_127 << 96);
  append(gbtWord);
}

void GBTDecoder::finalize()
{
  for (auto& e : mElinks) {
    e.finalize();
  }
}

void GBTDecoder::printStatus(int maxelink) const
{
  std::cout << fmt::format("GBTDecoder({}) # GBT words seen {}\n", mId, mNofGBTWordsSeens);
  auto n = mElinks.size();
  if (maxelink > 0) {
    n = maxelink;
  }
  for (int i = 0; i < n; i++) {
    const auto& e = mElinks[i];
    std::cout << e << "\n";
  }
}
