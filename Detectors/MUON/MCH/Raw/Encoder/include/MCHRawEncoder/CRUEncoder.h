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

#include <cstdlib>
#include <vector>
#include "MCHRawCommon/SampaCluster.h"

namespace o2::mch::raw
{

/// @brief A CRUEncoder manages 24 GBTEncoder to encode the data of one CRU.
///
/// Data is added using the addChanngelData() method.
/// Then it can be exported using the moveToBuffer() method.
/// \nosubgrouping

class CRUEncoder
{
 public:
  virtual ~CRUEncoder() {}

  /** @name Main interface.
    */
  ///@{
  /// add data for one channel, identified by {solarId,groupId,chId}
  /// (where (solarId,groupId) is equivalent to (deId,dsId) somehow)
  ///
  /// \param solarId is the (absolute) identifier of a solar card
  /// \param elinkId is the index of the GBT elink used 0..39
  /// \param chId sampa channel id 0..31
  /// \param data the actual data to be added
  virtual void addChannelData(uint16_t solarId, uint8_t elinkId, uint8_t chId, const std::vector<SampaCluster>& data) = 0;

  // startHeartbeatFrame sets the trigger (orbit,bunchCrossing) to be used
  // for all generated RDHs (until next call to this method).
  // Causes the alignment of the underlying gbts.
  virtual void startHeartbeatFrame(uint32_t orbit, uint16_t bunchCrossing) = 0;

  /// Export our encoded data.
  ///
  /// The internal words that have been accumulated so far are
  /// _moved_ (i.e. deleted from this object) to the external buffer of bytes
  /// Returns the number of bytes added to buffer.
  virtual size_t moveToBuffer(std::vector<uint8_t>& buffer) = 0;
  ///@}
};

} // namespace o2::mch::raw

#endif
