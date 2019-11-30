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
#include <gsl/span>
#include <iostream>
#include <map>
#include <vector>
#include <set>

namespace o2::mch::raw
{

/// @brief (Default) implementation of CRUEncoder
///
/// \nosubgrouping

template <typename FORMAT, typename CHARGESUM, typename RDHTYPE>
class CRUEncoderImpl : public CRUEncoder
{

 public:
  explicit CRUEncoderImpl(uint16_t cruId, std::set<uint16_t> solarIds);

  void addChannelData(uint16_t solarId, uint8_t groupId, uint8_t chId, const std::vector<SampaCluster>& data) override;

  void startHeartbeatFrame(uint32_t orbit, uint16_t bunchCrossing) override;

  size_t moveToBuffer(std::vector<uint8_t>& buffer) override;

 private:
  void closeHeartbeatFrame(uint32_t orbit, uint16_t bunchCrossing);
  void gbts2buffer(uint32_t orbit, uint16_t bunchCrossing);

 private:
  uint16_t mCruId;
  uint32_t mOrbit;
  uint16_t mBunchCrossing;
  std::vector<uint8_t> mBuffer;
  std::array<GBTEncoder<FORMAT, CHARGESUM>, 24> mGBTs;
  std::vector<uint16_t> mSolarIds;
  bool mFirstHBFrame;
};

template <typename FORMAT, typename CHARGESUM, typename RDH>
CRUEncoderImpl<FORMAT, CHARGESUM, RDH>::CRUEncoderImpl(uint16_t cruId, std::set<uint16_t> solarIds)
  : mCruId(cruId),
    mOrbit{},
    mBunchCrossing{},
    mBuffer{},
    mGBTs{impl::makeArray<24>([cruId](size_t i) { return GBTEncoder<FORMAT, CHARGESUM>(i); })},
    mFirstHBFrame{true},
    mSolarIds(solarIds.begin(), solarIds.end())
{
  impl::assertIsInRange("cruId", cruId, 0, 0xFFF); // 12 bits for cruId
}

template <typename FORMAT, typename CHARGESUM, typename RDH>
void CRUEncoderImpl<FORMAT, CHARGESUM, RDH>::addChannelData(uint16_t solarId, uint8_t groupId, uint8_t chId, const std::vector<SampaCluster>& data)
{
  auto ix = std::find(mSolarIds.begin(), mSolarIds.end(), solarId);
  if (ix == mSolarIds.end()) {
    throw std::invalid_argument(fmt::format("solarId {} is not known to CRU {}\n", solarId, mCruId));
  }
  auto solarIndex = std::distance(mSolarIds.begin(), ix);
  mGBTs.at(solarIndex).addChannelData(groupId, chId, data);
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
    impl::assertIsInRange("payloadSize", payloadSize, 0, (static_cast<uint32_t>(1) << 31) - 64);
    auto rdh = createRDH<RDH>(mCruId, gbt.id(), orbit, bunchCrossing, payloadSize);
    // append RDH first ...
    appendRDH(mBuffer, rdh);
    // ... and then the corresponding payload
    std::copy(gbtBuffer.begin(), gbtBuffer.end(), std::back_inserter(mBuffer));
  }
}

template <typename FORMAT, typename CHARGESUM, typename RDH>
size_t CRUEncoderImpl<FORMAT, CHARGESUM, RDH>::moveToBuffer(std::vector<uint8_t>& buffer)
{
  closeHeartbeatFrame(mOrbit, mBunchCrossing);
  std::copy(mBuffer.begin(), mBuffer.end(), std::back_inserter(buffer));
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
