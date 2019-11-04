// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_BARE_GBT_ENCODER_H
#define O2_MCH_RAW_BARE_GBT_ENCODER_H

#include <array>
#include "BareElinkEncoder.h"
#include <boost/multiprecision/cpp_int.hpp>

namespace o2
{
namespace mch
{
namespace raw
{

typedef boost::multiprecision::uint128_t uint128_t;

/// @brief A BareGBTEncoder manages 40 ElinkEncoder to encode the data of one GBT.
///
/// Channel data is added using the addChannelData() method.
/// The encoded data (in the form of 80-bits GBT words)
/// is exported to 32-bits words buffer using the moveToBuffer() method.
///
/// \nosubgrouping

class BareGBTEncoder
{
 public:
  /// Constructor.
  /// \param cruId the CRU this GBT is part of
  /// \param gbtId the id of this GBT. _Must_ be between 0 and 23
  /// or an exception is thrown
  /// \param chargeSumMode must be true if the data to be generated is
  /// in clusterMode
  BareGBTEncoder(int cruId, int gbtId, bool chargeSumMode = true);

  /** @name Main interface.
    */
  ///@{
  /// add data for one channel.
  ///
  /// \param elinkId 0..39
  /// \param chId 0..31
  /// \param data vector of SampaCluster objects
  void addChannelData(uint8_t elinkId, uint8_t chId, const std::vector<SampaCluster>& data);

  /// reset local bunch-crossing counter.
  ///
  /// (the one that is used in the sampa headers)
  void resetLocalBunchCrossing();

  /// Export our encoded data.
  ///
  /// The internal GBT words that have been accumulated so far are
  /// _moved_ (i.e. deleted from this object) to the external buffer of bytes.
  /// Returns the number of bytes added to buffer
  size_t moveToBuffer(std::vector<uint8_t>& buffer);
  ///@}

  /** @name Methods for testing.
    */
  ///@{
  /// Print the current status of the encoder, for as much as maxelink elinks.
  void printStatus(int maxelink = -1) const;

  /// Sets to true to bypass simulation of time misalignment of elinks.
  static bool forceNoPhase;
  ///@}

  /// returns the GBT id
  int id() const { return mGbtId; }

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
  int mCruId;                               //< CRU this GBT belongs to
  int mGbtId;                               //< id of this GBT (0..23)
  std::array<BareElinkEncoder, 40> mElinks; //< the 40 Elinks we manage
  std::vector<uint128_t> mGbtWords;         //< the (80 bits) GBT words we've accumulated so far
};
} // namespace raw
} // namespace mch
} // namespace o2
#endif
