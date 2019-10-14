// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_GBT_DECODER_H
#define O2_MCH_RAW_GBT_DECODER_H

#include <array>
#include "MCHRaw/ElinkDecoder.h"
#include "MCHRaw/SampaChannelHandler.h"
#include "MCHRaw/GBT.h"
#include <gsl/span>

namespace o2
{
namespace mch
{
namespace raw
{

/// @brief A GBTDecoder groups 40 ElinkDecoder objects.

class GBTDecoder
{
 public:
  /// Constructor.
  /// \param cruId the identifier for the CRU this GBT is part of
  /// \param sampaChannelHandler the callable that will handle each SampaCluster
  /// \param chargeSumMode whether the Sampa is in clusterMode or not
  GBTDecoder(int cruId, int gbtId, SampaChannelHandler sampaChannelHandler,
             bool chargeSumMode = true);

  /** @name Main interface 
    */
  ///@{

  /** @brief Append one GBT word (128 bits, split in 4 32 bits words).
    *
    * Note that for mch data only 80 out of the 128 are of real interest.
    */
  void append(uint32_t w0, uint32_t w1, uint32_t w2, uint32_t w3);
  ///@}

  /** @name Methods for testing
    */

  ///@{

  /// printStatus shows the status of some elinks (all if maxelink=-1)
  void printStatus(int maxelink = -1) const;

  /// Ensure any leftover data is properly decoded.
  void finalize();

  /// Clear our internal Elinks
  void reset();
  ///@}

 private:
  void append(uint128_t w);

 private:
  int mCruId;
  int mGbtId;
  std::array<ElinkDecoder, 40> mElinks;
  int mNofGbtWordsSeens;
};
} // namespace raw
} // namespace mch
} // namespace o2
#endif
