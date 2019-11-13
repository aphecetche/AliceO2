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

namespace o2
{
namespace mch
{
namespace raw
{

/// @brief A BareGBTDecoder groups 40 ElinkDecoder objects.

class BareGBTDecoder
{
 public:
  /// Constructor.
  /// \param cruId the identifier for the CRU this GBT is part of
  /// \param sampaChannelHandler the callable that will handle each SampaCluster
  /// \param chargeSumMode whether the Sampa is in clusterMode or not
  BareGBTDecoder(int cruId, int gbtId, SampaChannelHandler sampaChannelHandler,
                 bool chargeSumMode = true);

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
  std::array<BareElinkDecoder, 40> mElinks;
  int mNofGbtWordsSeens;
};
} // namespace raw
} // namespace mch
} // namespace o2
#endif
