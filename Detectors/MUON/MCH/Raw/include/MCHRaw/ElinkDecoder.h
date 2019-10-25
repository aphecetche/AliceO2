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

#include "SampaHeader.h"
#include <iostream>
#include <functional>
#include "MCHRaw/SampaChannelHandler.h"
#include <vector>

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
  void append(bool bit0, bool bit1);
  ///@}

  /// linkId is the GBT id this Elink is part of
  uint8_t linkId() const;

  /** @name Methods for testing
    */
  ///@{

  /// Current number of bits we're holding
  int len() const;

  /// Reset our internal bit stream, and the sync status
  /// i.e. assume the sync has to be found again
  void reset();
  ///@}

 private:
  /// The possible states we can be in
  enum class State : int {
    LookingForSync,    //< we've not found a sync yet
    LookingForHeader,  //< we've looking for a 50-bits header
    ReadingNofSamples, //< we're (about to) read nof of samples
    ReadingTimestamp,  //< we're (about to) read a timestamp (for the current cluster)
    ReadingSample,     //< we're (about to) read a sample (for the current cluster)
    ReadingClusterSum  //< we're (about to) read a chargesum (for the current cluster)
  };

  std::string name(State state) const;
  void changeState(State newState, int newCheckpoint);
  void clear(int checkpoint);
  void findSync();
  void handlReadClusterSum();
  void handleHeader();
  void handleReadClusterSum();
  void handleReadData();
  void handleReadSample();
  void handleReadTimestamp();
  void oneLess10BitWord();
  void process();
  void sendCluster();
  void softReset();

  friend std::ostream& operator<<(std::ostream& os, const ElinkDecoder& e);

 private:
  uint8_t mCruId;                           //< Identifier of the CRU this Elink is part of
  uint8_t mLinkId;                          //< Identifier of this Elink (0..39)
  bool mClusterSumMode;                     //< Whether we should expect 20-bits data words
  SampaChannelHandler mSampaChannelHandler; //< The callable that will deal with the SampaCluster objects we decode
  SampaHeader mSampaHeader;                 //< Current SampaHeader
  uint64_t mBitBuffer;                      //< Our internal bit stream buffer
  /** @name internal global counters
    */

  ///@{
  uint64_t mNofSync;               //< Number of SYNC words we've seen so far
  uint64_t mNofBitSeen;            //< Total number of bits seen
  uint64_t mNofHeaderSeen;         //< Total number of headers seen
  uint64_t mNofHammingErrors;      //< Total number of hamming errors seen
  uint64_t mNofHeaderParityErrors; //< Total number of header parity errors seen
  ///@}

  uint64_t mCheckpoint;           //< mask of the next state transition check to be done in process()
  uint16_t mNof10BitsWordsToRead; //< number of 10 bits words to be read

  uint16_t mNofSamples;
  uint16_t mTimestamp;
  std::vector<uint16_t> mSamples;
  uint32_t mClusterSum;
  uint64_t mMask;

  State mState; //< the state we are in
};

} // namespace raw
} // namespace mch
} // namespace o2

#endif
