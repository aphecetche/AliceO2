// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_USERLOGIC_GBT_DECODER_H
#define O2_MCH_RAW_USERLOGIC_GBT_DECODER_H

#include <array>
#include "UserLogicDSDecoder.h"
#include "MCHRawDecoder/SampaChannelHandler.h"
#include <gsl/span>

namespace o2
{
namespace mch
{
namespace raw
{

/// @brief A UserLogicGBTDecoder groups 40 UserLogicChannelDecoder objects.

class UserLogicGBTDecoder
{
 public:
  /// Constructor.
  /// \param cruId the identifier for the CRU this GBT is part of
  /// \param sampaChannelHandler the callable that will handle each SampaCluster
  /// \param chargeSumMode whether the Sampa is in clusterMode or not
  UserLogicGBTDecoder(int cruId, int gbtId, SampaChannelHandler sampaChannelHandler,
                      bool chargeSumMode = true);

  /** @name Main interface 
    */
  ///@{

  /** @brief Append the equivalent n 64-bits words 
    * bytes size (=n) must be a multiple of 8
    */
  void append(gsl::span<uint8_t> bytes);
  ///@}

  /** @name Methods for testing
    */

  ///@{

  /// printStatus shows the status of some elinks (all if maxelink=-1)
  void printStatus(int maxelink = -1) const;

  /// Clear our internal Elinks
  void reset();
  ///@}

 private:
  int mCruId;
  int mGbtId;
  std::array<UserLogicDSDecoder, 40> mDSDecoders;
  int mNofGbtWordsSeens;
};
} // namespace raw
} // namespace mch
} // namespace o2
#endif
