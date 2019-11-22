// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_BARE_GBT_DECODER_H
#define O2_MCH_RAW_BARE_GBT_DECODER_H

#include <array>
#include "BareElinkDecoder.h"
#include "MCHRawDecoder/SampaChannelHandler.h"
#include <gsl/span>
#include <fmt/printf.h>
#include <fmt/format.h>
#include "MakeArray.h"
#include "Assertions.h"
#include <iostream>
#include <boost/multiprecision/cpp_int.hpp>

namespace o2
{
namespace mch
{
namespace raw
{

/// @brief A BareGBTDecoder groups 40 ElinkDecoder objects.

template <typename CHARGESUM>
class BareGBTDecoder
{
 public:
  static constexpr uint8_t baseSize{128};

  /// Constructor.
  /// \param cruId the identifier for the CRU this GBT is part of
  /// \param sampaChannelHandler the callable that will handle each SampaCluster
  /// \param chargeSumMode whether the Sampa is in clusterMode or not
  BareGBTDecoder(int cruId, int gbtId, SampaChannelHandler sampaChannelHandler);

  /** @name Main interface 
    */
  ///@{

  /** @brief Append the equivalent n GBT words 
    * (n x 128 bits, split in 16 bytes).
    * bytes size (=n) must be a multiple of 16
    * Given that the MCH data only uses 80 out of the 128 bits
    * only the 10 first bytes of each group of 16 are used
    */
  void append(gsl::span<uint8_t> bytes);
  ///@}

  /** @name Methods for testing
    */

  ///@{

  /// Clear our internal Elinks
  void reset();
  ///@}

 private:
  void append(uint32_t, int, int);

 private:
  int mCruId;
  int mGbtId;
  std::array<BareElinkDecoder<CHARGESUM>, 40> mElinks;
  int mNofGbtWordsSeens;
};

using namespace boost::multiprecision;

template <typename CHARGESUM>
BareGBTDecoder<CHARGESUM>::BareGBTDecoder(int cruId,
                                          int gbtId,
                                          SampaChannelHandler sampaChannelHandler)
  : mCruId(cruId),
    mGbtId(gbtId),
    mElinks{impl::makeArray<40>([=](size_t i) { return BareElinkDecoder<CHARGESUM>(cruId, i, sampaChannelHandler); })},
    mNofGbtWordsSeens{0}
{
  impl::assertIsInRange("gbtId", gbtId, 0, 23);
}

template <typename CHARGESUM>
void BareGBTDecoder<CHARGESUM>::append(gsl::span<uint8_t> bytes)
{
  if (bytes.size() % 16 != 0) {
    throw std::invalid_argument("can only bytes by group of 16 (i.e. 128 bits)");
  }
  for (int j = 0; j < bytes.size(); j += 16) {
    if (j % 16 > 10) {
      // only consider 80 bits of each 128 bits group
      continue;
    }
    ++mNofGbtWordsSeens;
    int elinkIndex = 0;
    for (auto b : bytes.subspan(j, 10)) {
      mElinks[elinkIndex++].append(b & 2, b & 1);
      mElinks[elinkIndex++].append(b & 8, b & 4);
      mElinks[elinkIndex++].append(b & 32, b & 16);
      mElinks[elinkIndex++].append(b & 128, b & 64);
    }
  }
}

template <typename CHARGESUM>
void BareGBTDecoder<CHARGESUM>::reset()
{
  for (auto& e : mElinks) {
    e.reset();
  }
}
} // namespace raw
} // namespace mch
} // namespace o2

#endif
