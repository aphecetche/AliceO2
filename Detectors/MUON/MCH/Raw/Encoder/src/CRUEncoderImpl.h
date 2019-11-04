// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_CRU_ENCODER_IMPL_H
#define O2_MCH_RAW_CRU_ENCODER_IMPL_H

#include <cstdlib>
#include <vector>
#include <map>
#include "MCHRawEncoder/CRUEncoder.h"

#include <iostream>
#include <fmt/format.h>

namespace o2::mch::raw
{

/// @brief A CRUEncoderImpl manages 24 GBTEncoder to encode the data of one CRU.
///
/// Data is added using the addChannelData() method.
/// Then it can be exported using the moveToBuffer() method.
///
/// \nosubgrouping

template <typename GBTENCODER, typename RDHTYPE>
class CRUEncoderImpl : public CRUEncoder
{

 public:
  /// Constructor.
  /// \param cruId the CRU we're encoding data for.
  /// \param chargeSumMode must be true if the data to be generated is
  /// in clusterMode
  CRUEncoderImpl(uint16_t cruId, bool chargeSumMode = true);

  /** @name Main interface.
    */
  ///@{
  /// add data for one channel, identified by {solarId,elinkId,chId}
  /// \param solarId aka GBTId 0..23
  /// \param elinkId the linkId within the GBT
  /// \param chId channel id
  /// \param data the actual data to be added
  void addChannelData(uint8_t solarId, uint8_t elinkId, uint8_t chId, const std::vector<SampaCluster>& data);

  // startHeartbeatFrame sets the trigger (orbit,bunchCrossing) to be used
  // for all generated RDHs (until next call to this method).
  // Causes the alignment of the underlying gbts.
  void startHeartbeatFrame(uint32_t orbit, uint16_t bunchCrossing);

  /// Export our encoded data.
  ///
  /// The internal words that have been accumulated so far are
  /// _moved_ (i.e. deleted from this object) to the external buffer of bytes
  /// Returns the number of bytes added to buffer.
  size_t moveToBuffer(std::vector<uint8_t>& buffer);
  ///@}

  /** @name Methods for testing.
    */
  ///@{
  /// Print the current status of the encoder, for as much as maxelink elinks.
  void printStatus(int maxgbt = -1) const;
  ///@}

 private:
  void closeHeartbeatFrame(uint32_t orbit, uint16_t bunchCrossing);
  void gbts2buffer(uint32_t orbit, uint16_t bunchCrossing);

 private:
  uint16_t mCruId;
  uint32_t mOrbit;
  uint16_t mBunchCrossing;
  std::vector<uint32_t> mBuffer;
  std::array<GBTENCODER, 24> mGBTs;
  bool mFirstHBFrame;
};

} // namespace o2::mch::raw

#include "CRUEncoderImpl.inl"

#endif
