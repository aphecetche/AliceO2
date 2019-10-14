// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_ELINK_DECODER_H
#define O2_MCH_RAW_ELINK_DECODER_H

#include "BitSet.h"
#include "SampaHeader.h"
#include <iostream>
#include <functional>
#include "MCHRaw/SampaChannelHandler.h"

namespace o2
{
namespace mch
{
namespace raw
{

/// @brief Main element of the MCH Bare Raw Data Format decoder.
///
/// An ElinkDecoder manages the bit stream for one Elink.
///
/// Bits coming from parts of the GBT words are added to the Elink using the
/// append() method  and each time a SampaCluster is decoded,
/// it is passed to the SampaChannelHandler for further processing (or none).
///
/// \nosubgrouping

class ElinkDecoder
{
 public:
  /// Constructor.
  /// \param cruId cru this Elink is part of
  /// \param linkId the identifier of this Elink 0..39. If not within range, ctor will throw.
  /// \param sampaChannelHandler a callable that is passed each SampaCluster that will be decoded
  /// \param chargeSumMode whether or not the Sampa is in clusterSum mode
  ElinkDecoder(uint8_t cruId, uint8_t linkId, SampaChannelHandler sampaChannelHandler, bool chargeSumMode = true);

  /** @name Main interface 
  */
  ///@{

  /// Append two bits (from the same dual sampa, one per sampa) to the Elink.
  bool append(bool bit0, bool bit1);
  ///@}

  /// linkId is the GBT id this Elink is part of
  uint8_t linkId() const;

  /** @name Methods for testing
    */
  ///@{

  /// Ensure any leftover data in the bit stream is actually processed now
  bool finalize();

  /// Current number of bits we're holding
  int len() const;

  /// Clear our internal bit stream
  void reset();
  ///@}

 private:
  bool process();
  void clear(int checkpoint);
  void findSync();
  bool getData();
  void handlePacket10();
  void handlePacket20();
  bool append(bool bit);
  friend std::ostream& operator<<(std::ostream& os, const ElinkDecoder& e);

 private:
  uint8_t mCruId;  //< Identifier of the CRU this Elink is part of
  uint8_t mLinkId; //< Identifier of this Elink (0..39)
  int mCheckpoint; //< Index (in the bitset) of the next state transition check to be done in process()
  bool mIsInData;  //< Whether or not we are in the middle of data bits
  int mNofSync;    //< Number of SYNC words we've seen so far
  BitSet mBitSet;  //< Our internal bit stream buffer (is not growing indefinitely but cleared as soon as possible)
  BitSet mTotal;
  SampaHeader mSampaHeader;                 //< Current SampaHeader
  uint64_t mNofBitSeen;                     //< Total number of bits seen
  uint64_t mNofHeaderSeen;                  //< Total number of headers seen
  SampaChannelHandler mSampaChannelHandler; //< The callable that will deal with the SampaCluster objects we decode
  bool mChargeSumMode;                      //< Whether we should expect 20-bits data words
};

} // namespace raw
} // namespace mch
} // namespace o2

#endif
