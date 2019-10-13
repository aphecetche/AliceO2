// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHRaw/CRUEncoder.h"
#include "Assertions.h"
#include "MCHRaw/RAWDataHeader.h"
#include "MakeArray.h"
#include <algorithm>
#include <fmt/format.h>

namespace o2
{
namespace mch
{
namespace raw
{

using ::operator<<;

CRUEncoder::CRUEncoder(uint16_t cruId, bool chargeSumMode)
  : mCruId(cruId),
    mOrbit{},
    mBunchCrossing{},
    mBuffer{},
    mGBTs{::makeArray<24>([cruId, chargeSumMode](size_t i) { return GBTEncoder(cruId, i, chargeSumMode); })}
{
  assertIsInRange("cruId", cruId, 0, 0xFFF); // 12 bits for cruId
  mBuffer.reserve(1 << 10);
}

void CRUEncoder::addChannelData(uint8_t solarId, uint8_t elinkId, uint8_t chId, const std::vector<SampaCluster>& data)
{
  mGBTs[solarId].addChannelData(elinkId, chId, data);
}

void CRUEncoder::gbts2buffer(uint32_t orbit, uint16_t bunchCrossing)
{
  // append to our own buffer all the words buffers from all our gbts,
  // prepending each one with a corresponding Raw Data Header (RDH)

  for (auto& gbt : mGBTs) {
    std::vector<uint32_t> gbtBuffer;
    gbt.moveToBuffer(gbtBuffer);
    if (!gbtBuffer.size()) {
      // unlike in real life we discard gbt content when
      // it's completely void of data
      continue;
    }
    auto payloadSize = gbtBuffer.size() * sizeof(gbtBuffer[0]); // in bytes
    auto rdh = createRDH(mCruId, gbt.id(), orbit, bunchCrossing, payloadSize);
    // append RDH first ...
    appendRDH(mBuffer, rdh);
    // ... and then the corresponding payload
    std::copy(begin(gbtBuffer), end(gbtBuffer), std::back_inserter(mBuffer));
  }
}

size_t CRUEncoder::moveToBuffer(std::vector<uint32_t>& buffer)
{
  closeHeartbeatFrame(mOrbit, mBunchCrossing);
  for (auto& w : mBuffer) {
    buffer.emplace_back(w);
  }
  auto s = mBuffer.size();
  mBuffer.clear();
  return s;
}

void CRUEncoder::printStatus(int maxgbt) const
{
  std::cout << fmt::format("CRUEncoder({}) buffer size {}\n", mCruId, mBuffer.size());

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

void CRUEncoder::closeHeartbeatFrame(uint32_t orbit, uint16_t bunchCrossing)
{
  if (orbit && bunchCrossing) {
    gbts2buffer(orbit, bunchCrossing);
  }
}

void CRUEncoder::startHeartbeatFrame(uint32_t orbit, uint16_t bunchCrossing)
{
  assertIsInRange("bunchCrossing", bunchCrossing, 0, 0xFFF);
  // build a buffer with the _previous_ (orbit,bx)
  closeHeartbeatFrame(mOrbit, mBunchCrossing);
  // then save the (orbit,bx) for next time
  mOrbit = orbit;
  mBunchCrossing = bunchCrossing;
}

} // namespace raw
} // namespace mch
} // namespace o2
