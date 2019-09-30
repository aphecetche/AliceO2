// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHRaw/GBTEncoder.h"
#include <fmt/printf.h>
#include <stdexcept>

using namespace o2::mch::raw;

namespace
{
template <typename CTOR, size_t... S>
std::array<std::invoke_result_t<CTOR, size_t>, sizeof...(S)> makeArray(CTOR&& ctor,
                                                                       std::index_sequence<S...>)
{
  return std::array<std::invoke_result_t<CTOR, size_t>, sizeof...(S)>{{ctor(S)...}};
}

template <size_t N, typename CTOR>
std::array<std::invoke_result_t<CTOR, size_t>, N> makeArray(CTOR&& ctor)
{
  return makeArray(std::forward<CTOR>(ctor), std::make_index_sequence<N>());
}

} // namespace

// FIXME: instead of i % 16 for elinkid , use 0..39 and let the elinkencoder compute the dsid within
// the right range 0..15 itself ? Or have the mapping at this level already ?
GBTEncoder::GBTEncoder(int linkId) : mId(linkId), mElinks{::makeArray<40>([](size_t i) { return ElinkEncoder(i % 16); })}, mGBTWords{}, mElinksInSync{true}
{
  if (linkId < 0 || linkId > 23) {
    throw std::invalid_argument(fmt::sprintf("linkId %d should be between 0 and 23", linkId));
  }
}

void GBTEncoder::addChannelChargeSum(uint32_t bx, uint8_t elinkId, uint8_t chId, uint16_t timestamp, uint32_t chargeSum)
{
  mElinksInSync = false;
  if (elinkId < 0 || elinkId > 39) {
    throw std::invalid_argument(fmt::sprintf("elinkId %d should be between 0 and 39", elinkId));
  }
  mElinks[elinkId].bunchCrossingCounter(bx);
  mElinks[elinkId].addChannelChargeSum(chId, timestamp, chargeSum);
}

void GBTEncoder::elink2gbt()
{
  // convert elinks content to actual GBT words
  if (!mElinksInSync) {
    throw std::logic_error("should not call elink2gbt before all elinks are synched !");
  }

  int n = mElinks[0].len();
  uint80_t one{1};

  for (int i = 0; i < n; i++) {
    uint80_t w{0};
    for (int j = 0; j < 40; j++) {
      uint80_t mask = one << j;
      bool v = mElinks[j].get(i);
      if (v) {
        bit_set(w, j);
      } else {
        bit_unset(w, j);
        ;
      }
    }
    mGBTWords.push_back(w);
  }
}

void GBTEncoder::finalize()
{
  if (mElinksInSync) {
    return;
  }

  // find the elink which has the more bits
  auto e = std::max_element(begin(mElinks), end(mElinks),
                            [](const ElinkEncoder& a, const ElinkEncoder& b) {
                              return a.len() < b.len();
                            });

  // align all elink sizes to the biggest one by adding
  // sync words
  for (auto i = 0; i < mElinks.size(); i++) {
    mElinks[i].fillWithSync(e->len());
  }

  // signals that the elinks have now the same size
  mElinksInSync = true;

  // convert elinks to GBT words
  elink2gbt();

  // clear the elinks
  for (auto i = 0; i < mElinks.size(); i++) {
    mElinks[i].clear();
  }
}

size_t GBTEncoder::size() const
{
  return mGBTWords.size();
}

GBTEncoder::uint80_t GBTEncoder::getWord(int i) const
{
  return mGBTWords[i];
}
