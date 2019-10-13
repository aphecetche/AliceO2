// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_ELINK_ENCODER_H
#define O2_MCH_RAW_ELINK_ENCODER_H

#include "BitSet.h"
#include "SampaHeader.h"
#include <vector>
#include <iostream>
#include <gsl/span>
#include "MCHRaw/SampaCluster.h"

namespace o2
{
namespace mch
{
namespace raw
{

/// @brief Center piece of the MCH Bare Raw Data Format encoder logic.
///
/// Converts the data from one SampaCluster into a bit stream
/// that mimics the way Elinks see data.

class ElinkEncoder
{
 public:
  /// Constructs an Encoder for one Elink.
  ///
  /// \param id is the elink identifier and _must_ be between 0 and 39
  /// \param chip is the sampa chip number this encoder is dealing withn and
  /// _must_ be between 0 and 15.
  /// \param phase can be used to simulate a different time alignment
  /// between elinks
  /// \param chargeSumMode must be true if the data to be generated is
  /// in clusterMode
  ///
  /// if elinkId or chip are not within allowed range an exception
  /// is thrown.
  explicit ElinkEncoder(uint8_t elinkId, uint8_t chip, int phase = 0,
                        bool chargeSumMode = true);

  /// addChannelData converts the SampaCluster data into a bit sequence.
  ///
  /// \param chId is the Sampa channel to associated this data with
  /// \param data is a vector of SampaCluster representing the SampaCluster(s)
  /// of this channel within one Sampa time window.
  void addChannelData(uint8_t chId, const std::vector<SampaCluster>& data);

  /// Empty the bit stream.
  void clear();

  /// fillWithSync appends bits from the sync word until the length
  /// is `upto`
  void fillWithSync(int upto);

  /// get the i-th bit in our bit stream.
  bool get(int i) const;

  /// id is the elink id (0..39)
  uint8_t id() const;

  /// len returns the number of bits we have in our bit stream
  int len() const;

  /// reset our local bunch crossing counter
  void resetLocalBunchCrossing();

  friend std::ostream& operator<<(std::ostream& os, const ElinkEncoder& enc);

  /// converts the bits within a range into an integer value.
  /// throws if the range [a,b] does not fit within 64 bits.
  uint64_t range(int a, int b) const;

 private:
  void append(bool value);
  void append(const SampaCluster& sc);
  void append10(uint16_t value);
  void append20(uint32_t value);
  void append50(uint64_t value);
  void assertNotMixingClusters(const std::vector<SampaCluster>& data) const;
  void assertPhase();
  void assertSync();
  uint64_t nofSync() const { return mNofSync; }
  void setHeader(uint8_t chId, uint16_t n10);

 private:
  uint8_t mElinkId;             //< Elink id 0..39
  uint8_t mChipAddress;         //< chip address 0..15
  SampaHeader mSampaHeader;     //< current sampa header
  BitSet mBitSet;               //< bitstream
  uint64_t mNofSync;            //< number of sync words seen so far
  int mSyncIndex;               //< at which sync bit position should the next fillWithSync start
  uint64_t mNofBitSeen;         //< total number of bits seen so far
  int mPhase;                   //< initial number of bits
  uint32_t mLocalBunchCrossing; //< bunchcrossing to be used in header
  bool mChargeSumMode;          //< whether or not we should encode 20 or 10 bits for data
};

} // namespace raw
} // namespace mch
} // namespace o2
#endif
