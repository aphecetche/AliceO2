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

#include "MCHRawEncoder/CRUEncoder.h"
#include "Assertions.h"
#include "GBTEncoder.h"
#include "Headers/RAWDataHeader.h"
#include "MCHRawCommon/RDHManip.h"
#include "MakeArray.h"
#include <algorithm>
#include <cstdlib>
#include <fmt/format.h>
#include <iostream>
#include <map>
#include <vector>

namespace o2::mch::raw
{

/// @brief A CRUEncoderImpl manages 24 GBTEncoder to encode the data of one CRU.
///
/// Data is added using the addChannelData() method.
/// Then it can be exported using the moveToBuffer() method.
///
/// \nosubgrouping

template <typename FORMAT, typename CHARGESUM, typename RDHTYPE>
class CRUEncoderImpl : public CRUEncoder
{

 public:
  /// Constructor.
  /// \param cruId the CRU we're encoding data for.
  CRUEncoderImpl(uint16_t cruId);

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

 private:
  void closeHeartbeatFrame(uint32_t orbit, uint16_t bunchCrossing);
  void gbts2buffer(uint32_t orbit, uint16_t bunchCrossing);

 private:
  uint16_t mCruId;
  uint32_t mOrbit;
  uint16_t mBunchCrossing;
  std::vector<uint8_t> mBuffer;
  std::array<GBTEncoder<FORMAT, CHARGESUM>, 24> mGBTs;
  bool mFirstHBFrame;
};

template <typename FORMAT, typename CHARGESUM, typename RDH>
CRUEncoderImpl<FORMAT, CHARGESUM, RDH>::CRUEncoderImpl(uint16_t cruId)
  : mCruId(cruId),
    mOrbit{},
    mBunchCrossing{},
    mBuffer{},
    mGBTs{impl::makeArray<24>([cruId](size_t i) { return GBTEncoder<FORMAT, CHARGESUM>(i); })},
    mFirstHBFrame{true}
{
  impl::assertIsInRange("cruId", cruId, 0, 0xFFF); // 12 bits for cruId
  // mBuffer.reserve(1 << 12);
}

template <typename FORMAT, typename CHARGESUM, typename RDH>
void CRUEncoderImpl<FORMAT, CHARGESUM, RDH>::addChannelData(uint8_t solarId, uint8_t elinkId, uint8_t chId, const std::vector<SampaCluster>& data)
{
  mGBTs[solarId].addChannelData(elinkId, chId, data);
}

template <typename FORMAT, typename CHARGESUM, typename RDH>
void CRUEncoderImpl<FORMAT, CHARGESUM, RDH>::gbts2buffer(uint32_t orbit, uint16_t bunchCrossing)
{
  // append to our own buffer all the words buffers from all our gbts,
  // prepending each one with a corresponding Raw Data Header (RDH)

  for (auto& gbt : mGBTs) {
    std::vector<uint8_t> gbtBuffer;
    gbt.moveToBuffer(gbtBuffer);
    if (!gbtBuffer.size()) {
      // unlike in real life we discard gbt content when
      // it's completely void of data
      continue;
    }
    assert(gbtBuffer.size() % 4 == 0);
    auto payloadSize = gbtBuffer.size(); // in bytes
    auto rdh = createRDH<RDH>(mCruId, gbt.id(), orbit, bunchCrossing, payloadSize);
    // append RDH first ...
    appendRDH(mBuffer, rdh);
    // ... and then the corresponding payload
    for (auto i = 0; i < gbtBuffer.size(); i++) {
      mBuffer.emplace_back(gbtBuffer[i]);
    }
  }
}

template <typename FORMAT, typename CHARGESUM, typename RDH>
size_t CRUEncoderImpl<FORMAT, CHARGESUM, RDH>::moveToBuffer(std::vector<uint8_t>& buffer)
{
  closeHeartbeatFrame(mOrbit, mBunchCrossing);
  for (auto& w : mBuffer) {
    buffer.emplace_back(w);
    // buffer.emplace_back(static_cast<uint8_t>(w & 0x000000FF));
    // buffer.emplace_back(static_cast<uint8_t>((w & 0x0000FF00) >> 8));
    // buffer.emplace_back(static_cast<uint8_t>((w & 0x00FF0000) >> 16));
    // buffer.emplace_back(static_cast<uint8_t>((w & 0xFF000000) >> 24));
  }
  auto s = mBuffer.size();
  mBuffer.clear();
  return s;
}

template <typename FORMAT, typename CHARGESUM, typename RDH>
void CRUEncoderImpl<FORMAT, CHARGESUM, RDH>::closeHeartbeatFrame(uint32_t orbit, uint16_t bunchCrossing)
{
  gbts2buffer(orbit, bunchCrossing);
}

template <typename FORMAT, typename CHARGESUM, typename RDH>
void CRUEncoderImpl<FORMAT, CHARGESUM, RDH>::startHeartbeatFrame(uint32_t orbit, uint16_t bunchCrossing)
{
  impl::assertIsInRange("bunchCrossing", bunchCrossing, 0, 0xFFF);
  // build a buffer with the _previous_ (orbit,bx)
  if (!mFirstHBFrame) {
    closeHeartbeatFrame(mOrbit, mBunchCrossing);
  }
  mFirstHBFrame = false;
  // then save the (orbit,bx) for next time
  mOrbit = orbit;
  mBunchCrossing = bunchCrossing;
}

} // namespace o2::mch::raw
#endif
