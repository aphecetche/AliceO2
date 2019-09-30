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
#include "MakeArray.h"

using namespace o2::mch::raw;

// FIXME: instead of i % 16 for elinkid , use 0..39 and let the elinkencoder compute the dsid within
// the right range 0..15 itself ? Or have the mapping at this level already ?
GBTEncoder::GBTEncoder(int linkId) : mId(linkId), mElinks{::makeArray<40>([](size_t i) { return ElinkEncoder(i, i % 16); })}, mElinkHasAtLeastOneSync{false}, mGBTWords{}, mElinksInSync{true}
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
  if (!mElinkHasAtLeastOneSync[elinkId]) {
    mElinks[elinkId].addSync();
    mElinkHasAtLeastOneSync[elinkId] = true;
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

  for (int i = 0; i < n; i++) {
    GBTWord w{0};
    for (int j = 0; j < 40; j++) {
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
    int n = mElinks[i].fillWithSync(e->len());
    if (n > 50) {
      mElinkHasAtLeastOneSync[i] = true;
    }
  }

  // signals that the elinks have now the same size
  mElinksInSync = true;

  // convert elinks to GBT words
  elink2gbt();

  clear();
}

void GBTEncoder::clear()
{
  // clear the elinks
  for (auto i = 0; i < mElinks.size(); i++) {
    mElinks[i].clear();
    mElinkHasAtLeastOneSync[i] = false;
  }
}

size_t GBTEncoder::size() const
{
  return mGBTWords.size();
}

GBTWord GBTEncoder::getWord(int i) const
{
  return mGBTWords[i];
}
