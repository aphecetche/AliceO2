// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_GBT_ENCODER_H
#define O2_MCH_RAW_GBT_ENCODER_H

#include <array>
#include "MCHRaw/ElinkEncoder.h"
#include "MCHRaw/GBT.h"

namespace o2
{
namespace mch
{
namespace raw
{

class GBTEncoder
{
 public:
  GBTEncoder(int cruId, int gbtId);

  ///@{ Main interface
  /// add the integrated charge for one channel
  void addChannelChargeSum(uint8_t elinkId, uint16_t timestamp, uint8_t chId, uint32_t chargeSum);

  /// reset local bunch-crossing counter
  /// (the one that is used in the sampa headers)
  void resetLocalBunchCrossing();

  /// The internal GBT words that have been accumulated so far are
  /// _moved_ (i.e. deleted from this object) to the external buffer.
  /// Returns the number of words added to buffer.
  size_t moveToBuffer(std::vector<uint32_t>& buffer);
  ///@}

  ///@{ Methods mainly used for testing
  /// Print the current status of the encoder, for as much as maxelink elinks.
  void printStatus(int maxelink = -1) const;
  /// get the number of internal 32-bits words we have accumulated so far
  size_t nofWords() const;
  ///@}

 private:
  bool areElinksAligned() const;

  int len() const;

  size_t size() const;

  uint128_t getWord(int i) const;

  void align(int upto);

  void elink2gbt();

  void clear();

  void finalize(int alignToSize = 0);

 private:
  int mCruId;
  int mGbtId;
  std::array<ElinkEncoder, 40> mElinks;
  std::vector<uint128_t> mGbtWords;
};
} // namespace raw
} // namespace mch
} // namespace o2
#endif
