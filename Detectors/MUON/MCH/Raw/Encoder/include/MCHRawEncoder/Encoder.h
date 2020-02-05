// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_CRU_ENCODER_H
#define O2_MCH_RAW_CRU_ENCODER_H

#include "MCHRawEncoder/GBTEncoder.h"
#include <cstdlib>
#include <map>
#include <vector>
#include "MCHRawElecMap/DsElecId.h"
#include "MCHRawEncoder/DataBlock.h"

namespace o2::mch::raw
{

class DsElecId;
class SampaCluster;

/// @brief An Encoder builds MCH raw data
///
/// Data is added using the addChannelData() method.
/// Then it can be exported using the moveToBuffer() method.
/// \nosubgrouping

template <typename FORMAT, typename CHARGESUM>
class Encoder
{
 public:
  Encoder(bool forceNoPhase = false);

  /** @name Main interface.
    */
  ///@{
  /// add data for one channel.
  ///
  /// \param dsId is the (electronic) identifier of a dual sampa
  /// \param chId dual sampa channel id 0..63
  /// \param data the actual data to be added
  void addChannelData(DsElecId dsId, uint8_t chId, const std::vector<SampaCluster>& data);

  // startHeartbeatFrame sets the trigger (orbit,bunchCrossing) to be used
  // for all generated payload headers (until next call to this method).
  // Causes the alignment of the underlying gbts.
  void startHeartbeatFrame(uint32_t orbit, uint16_t bunchCrossing);

  /// Export our encoded data.
  ///
  /// The internal words that have been accumulated so far are
  /// _moved_ (i.e. deleted from this object) to the external buffer of bytes
  /// Returns the number of bytes added to buffer.
  size_t moveToBuffer(std::vector<uint8_t>& buffer);
  ///@}

 private:
  void closeHeartbeatFrame(uint32_t orbit, uint16_t bunchCrossing);
  void gbts2buffer(uint32_t orbit, uint16_t bunchCrossing);

 private:
  uint32_t mOrbit;
  uint16_t mBunchCrossing;
  std::vector<uint8_t> mBuffer;
  std::map<uint16_t, std::unique_ptr<GBTEncoder<FORMAT, CHARGESUM>>> mGBTs;
  bool mFirstHBFrame;
  bool mForceNoPhase;
};

} // namespace o2::mch::raw
#endif
