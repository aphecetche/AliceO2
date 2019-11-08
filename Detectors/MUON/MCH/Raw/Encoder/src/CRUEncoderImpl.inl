// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHRawEncoder/CRUEncoder.h"
#include "Assertions.h"
#include "Headers/RAWDataHeader.h"
#include "MCHRawCommon/RDHManip.h"
#include "MakeArray.h"
#include <algorithm>
#include <fmt/format.h>

namespace o2
{
namespace mch
{
namespace raw
{

template <typename GBTENCODER, typename RDH>
CRUEncoderImpl<GBTENCODER, RDH>::CRUEncoderImpl(uint16_t cruId, bool chargeSumMode)
  : mCruId(cruId),
    mOrbit{},
    mBunchCrossing{},
    mBuffer{},
    mGBTs{::makeArray<24>([cruId, chargeSumMode](size_t i) { return GBTENCODER(cruId, i, chargeSumMode); })},
    mFirstHBFrame{true}
{
  assertIsInRange("cruId", cruId, 0, 0xFFF); // 12 bits for cruId
  mBuffer.reserve(1 << 10);
}

template <typename GBTENCODER, typename RDH>
void CRUEncoderImpl<GBTENCODER, RDH>::addChannelData(uint8_t solarId, uint8_t elinkId, uint8_t chId, const std::vector<SampaCluster>& data)
{
  mGBTs[solarId].addChannelData(elinkId, chId, data);
}

template <typename GBTENCODER, typename RDH>
void CRUEncoderImpl<GBTENCODER, RDH>::gbts2buffer(uint32_t orbit, uint16_t bunchCrossing)
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
    for (auto i = 0; i < gbtBuffer.size(); i += 4) {
      mBuffer.emplace_back(gbtBuffer[i] | (gbtBuffer[i + 1] << 8) | (gbtBuffer[i + 2] << 16) | (gbtBuffer[i + 3] << 24));
    }
  }
}

template <typename GBTENCODER, typename RDH>
size_t CRUEncoderImpl<GBTENCODER, RDH>::moveToBuffer(std::vector<uint8_t>& buffer)
{
  closeHeartbeatFrame(mOrbit, mBunchCrossing);
  for (auto& w : mBuffer) {
    buffer.emplace_back(static_cast<uint8_t>(w & 0x000000FF));
    buffer.emplace_back(static_cast<uint8_t>((w & 0x0000FF00) >> 8));
    buffer.emplace_back(static_cast<uint8_t>((w & 0x00FF0000) >> 16));
    buffer.emplace_back(static_cast<uint8_t>((w & 0xFF000000) >> 24));
  }
  auto s = mBuffer.size();
  mBuffer.clear();
  return s;
}

template <typename GBTENCODER, typename RDH>
void CRUEncoderImpl<GBTENCODER, RDH>::printStatus(int maxgbt) const
{
  std::cout << fmt::format("CRUEncoderImpl({}) buffer size {}\n", mCruId, mBuffer.size());

  int n = mGBTs.size();
  if (maxgbt > 0 && maxgbt <= n) {
    n = maxgbt;
  }
  for (int i = 0; i < n; i++) {
    const auto& gbt = mGBTs[i];
    gbt.printStatus(5);
  }

  for (int i = 0; i < mBuffer.size(); i++) {
    std::cout << fmt::format("{:10X} ", mBuffer[i]);
    if ((i + 1) % 4 == 0) {
      std::cout << "\n";
    }
  }
}

template <typename GBTENCODER, typename RDH>
void CRUEncoderImpl<GBTENCODER, RDH>::closeHeartbeatFrame(uint32_t orbit, uint16_t bunchCrossing)
{
  gbts2buffer(orbit, bunchCrossing);
}

template <typename GBTENCODER, typename RDH>
void CRUEncoderImpl<GBTENCODER, RDH>::startHeartbeatFrame(uint32_t orbit, uint16_t bunchCrossing)
{
  assertIsInRange("bunchCrossing", bunchCrossing, 0, 0xFFF);
  // build a buffer with the _previous_ (orbit,bx)
  if (!mFirstHBFrame) {
    closeHeartbeatFrame(mOrbit, mBunchCrossing);
  }
  mFirstHBFrame = false;
  // then save the (orbit,bx) for next time
  mOrbit = orbit;
  mBunchCrossing = bunchCrossing;
}

} // namespace raw
} // namespace mch
} // namespace o2
