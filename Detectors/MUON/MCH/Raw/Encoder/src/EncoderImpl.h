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

#include "Assertions.h"
#include "GBTEncoder.h"
#include "Headers/RAWDataHeader.h"
#include "MCHRawCommon/RDHManip.h"
#include "MCHRawElecMap/Solar2CruMapper.h"
#include "MCHRawEncoder/Encoder.h"
#include "MakeArray.h"
#include <algorithm>
#include <cstdlib>
#include <fmt/format.h>
#include <functional>
#include <gsl/span>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <vector>

namespace o2::mch::raw
{

/// @brief (Default) implementation of Encoder
///
/// \nosubgrouping

template <typename FORMAT, typename CHARGESUM, typename RDHTYPE>
class EncoderImpl : public Encoder
{
 public:
  EncoderImpl(Solar2CruMapper solar2cru);

  void addChannelData(DsElecId dsId, uint8_t chId, const std::vector<SampaCluster>& data) override;

  void startHeartbeatFrame(uint32_t orbit, uint16_t bunchCrossing) override;

  size_t moveToBuffer(std::vector<uint8_t>& buffer) override;

 private:
  void closeHeartbeatFrame(uint32_t orbit, uint16_t bunchCrossing);
  void gbts2buffer(uint32_t orbit, uint16_t bunchCrossing);

 private:
  Solar2CruMapper mSolar2Cru;
  uint32_t mOrbit;
  uint16_t mBunchCrossing;
  std::vector<uint8_t> mBuffer;
  std::map<int, std::unique_ptr<GBTEncoder<FORMAT, CHARGESUM>>> mGBTs;
  bool mFirstHBFrame;
};

template <typename FORMAT, typename CHARGESUM, typename RDH>
EncoderImpl<FORMAT, CHARGESUM, RDH>::EncoderImpl(Solar2CruMapper solar2cru)
  : mSolar2Cru(solar2cru),
    mOrbit{},
    mBunchCrossing{},
    mBuffer{},
    mGBTs{},
    mFirstHBFrame{true}
{
}

template <typename FORMAT, typename CHARGESUM, typename RDH>
void EncoderImpl<FORMAT, CHARGESUM, RDH>::addChannelData(DsElecId dsId, uint8_t chId, const std::vector<SampaCluster>& data)
{
  auto solarId = dsId.solarId();
  auto gbt = mGBTs.find(solarId);
  if (gbt == mGBTs.end()) {
    mGBTs.emplace(solarId, std::make_unique<GBTEncoder<FORMAT, CHARGESUM>>(solarId));
  }
  mGBTs[solarId]->addChannelData(dsId.elinkGroupId(), dsId.elinkIndexInGroup(), chId, data);
}

template <typename FORMAT, typename CHARGESUM, typename RDH>
void EncoderImpl<FORMAT, CHARGESUM, RDH>::gbts2buffer(uint32_t orbit, uint16_t bunchCrossing)
{
  // append to our own buffer all the words buffers from all our gbts,
  // prepending each one with a corresponding Raw Data Header (RDH)

  for (auto& p : mGBTs) {
    auto& gbt = p.second;
    std::vector<uint8_t> gbtBuffer;
    gbt->moveToBuffer(gbtBuffer);
    if (!gbtBuffer.size()) {
      // unlike in real life we discard gbt content when
      // it's completely void of data
      continue;
    }
    assert(gbtBuffer.size() % 4 == 0);
    auto payloadSize = gbtBuffer.size(); // in bytes
    impl::assertIsInRange("payloadSize", payloadSize, 0, (static_cast<uint32_t>(1) << 31) - 64);
    auto solarId = gbt->id();
    auto cru = mSolar2Cru(solarId);
    uint16_t cruId = 0;
    if (!cru.has_value()) {
      std::cout << "WARNING : could not get cruId from solarId=" << solarId << " using dummy value = 0\n";
    } else {
      cruId = cru.value();
    }
    auto rdh = createRDH<RDH>(cruId, solarId, orbit, bunchCrossing, payloadSize);
    // append RDH first ...
    appendRDH(mBuffer, rdh);
    // ... and then the corresponding payload
    std::copy(gbtBuffer.begin(), gbtBuffer.end(), std::back_inserter(mBuffer));
  }
}

template <typename FORMAT, typename CHARGESUM, typename RDH>
size_t EncoderImpl<FORMAT, CHARGESUM, RDH>::moveToBuffer(std::vector<uint8_t>& buffer)
{
  closeHeartbeatFrame(mOrbit, mBunchCrossing);
  std::copy(mBuffer.begin(), mBuffer.end(), std::back_inserter(buffer));
  auto s = mBuffer.size();
  mBuffer.clear();
  return s;
}

template <typename FORMAT, typename CHARGESUM, typename RDH>
void EncoderImpl<FORMAT, CHARGESUM, RDH>::closeHeartbeatFrame(uint32_t orbit, uint16_t bunchCrossing)
{
  gbts2buffer(orbit, bunchCrossing);
}

template <typename FORMAT, typename CHARGESUM, typename RDH>
void EncoderImpl<FORMAT, CHARGESUM, RDH>::startHeartbeatFrame(uint32_t orbit, uint16_t bunchCrossing)
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
