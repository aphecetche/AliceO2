// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "BareElinkEncoder.h"
#include "ElinkEncoderMerger.h"

namespace o2::mch::raw
{

template <typename CHARGESUM>
bool areElinksAligned(const std::vector<ElinkEncoder<Bare, CHARGESUM>>& elinks)
{
  auto len = elinks[0].len();
  for (auto i = 1; i < elinks.size(); i++) {
    if (elinks[i].len() != len) {
      return false;
    }
  }
  return true;
}

template <typename CHARGESUM>
void align(std::vector<ElinkEncoder<Bare, CHARGESUM>>& elinks)
{
  if (areElinksAligned(elinks)) {
    return;
  }
  auto e = std::max_element(begin(elinks), end(elinks),
                            [](const ElinkEncoder<Bare, CHARGESUM>& a, const ElinkEncoder<Bare, CHARGESUM>& b) {
                              return a.len() < b.len();
                            });

  // align all elink sizes by adding sync bits
  for (auto& elink : elinks) {
    elink.fillWithSync(e->len());
  }
}

template <typename CHARGESUM>
void clear(std::vector<ElinkEncoder<Bare, CHARGESUM>>& elinks)
{
  // clear the elinks
  for (auto& elink : elinks) {
    elink.clear();
  }
}

template <typename CHARGESUM>
uint64_t aggregate(const std::vector<ElinkEncoder<Bare, CHARGESUM>>& elinks, int jstart, int jend, int i)
{
  uint64_t w{0};
  for (int j = jstart; j < jend; j += 2) {
    for (int k = 0; k <= 1; k++) {
      bool v = elinks[j / 2].get(i + 1 - k);
      uint64_t mask = static_cast<uint64_t>(1) << (j + k);
      if (v) {
        w |= mask;
      } else {
        w &= ~mask;
      }
    }
  }
  return w;
}

template <typename CHARGESUM>
void elink2gbt(const std::vector<ElinkEncoder<Bare, CHARGESUM>>& elinks, std::vector<uint64_t>& b64)
{
  int n = elinks[0].len();

  for (int i = 0; i < n - 1; i += 2) {
    uint64_t w0 = aggregate(elinks, 0, 64, i);
    uint64_t w1 = aggregate(elinks, 64, 80, i);
    b64.push_back(w0);
    b64.push_back(w1);
  }
}

template <>
void ElinkEncoderMerger(int gbtId,
                        gsl::span<ElinkEncoder<Bare, ChargeSumMode>> elinks,
                        gsl::span<uint64_t> b64)
{
  // align sizes of all elinks by adding sync bits
  align(elinks);

  // convert elinks to GBT words
  elink2gbt(elinks, b64);

  // reset all the links
  clear(elinks);
}

} // namespace o2::mch::raw
