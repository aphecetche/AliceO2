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

CRUEncoder::CRUEncoder(uint16_t cruId) : mId(cruId), mOrbit{}, mBunchCrossing{}, mBuffer{}, mGBTs{::makeArray<24>([cruId](size_t i) { return GBTEncoder(cruId, i); })}
{
  assertIsInRange("cruId", cruId, 0, 0xFFF); // 12 bits for cruId
  mBuffer.reserve(1 << 10);
}

void CRUEncoder::addChannelChargeSum(uint8_t solarId, uint8_t elinkId, uint8_t chId, uint16_t timestamp, uint32_t chargeSum)
{
  mGBTs[solarId].addChannelChargeSum(elinkId, chId, timestamp, chargeSum);
}

RAWDataHeader createRDH(uint16_t cruId, uint32_t orbit, uint16_t bunchCrossing,
                        uint16_t memorySize)
{
  RAWDataHeader rdh;

  rdh.cruId = cruId;
  rdh.dpwId = 0; // FIXME: fill this ?
  rdh.feeId = 0; //FIXME: what is this field supposed to contain ? unclear to me.
  rdh.priorityBit = 0;
  rdh.blockLength = memorySize; // FIXME: the blockLength disappears in RDHv5 ?
  rdh.memorySize = memorySize;
  rdh.packetCounter = 0; // FIXME: fill this ?
  rdh.triggerType = 0;   // FIXME: fill this ?
  rdh.detectorField = 0; // FIXME: fill this ?
  rdh.par = 0;           // FIXME: fill this ?
  rdh.stopBit = 0;
  rdh.pagesCounter = 1;
  rdh.triggerOrbit = orbit;
  rdh.heartbeatOrbit = orbit; // FIXME: RDHv5 has only triggerOrbit ?
  rdh.triggerBC = bunchCrossing;
  rdh.heartbeatBC = bunchCrossing; // FIXME: RDHv5 has only triggerBC ?

  return rdh;
}

void dumpRDH(const RAWDataHeader& rdh)
{
  std::cout << std::string(13, '-') << "RDH" << std::string(46, '-') << "\n";
  std::cout << rdh;
  std::cout << std::string(13 + 46 + strlen("RDH"), '-') << "\n";
}

void CRUEncoder::gbts2buffer(uint32_t orbit, uint16_t bunchCrossing)
{
  // get the words buffer from all our gbts, and prepend each one with a
  // RDH

  auto prev = mBuffer.size();

  for (auto& gbt : mGBTs) {
    std::vector<uint32_t> gbtBuffer;
    gbtBuffer.emplace_back(0x00000000);
    gbtBuffer.emplace_back(0x22222222);
    gbtBuffer.emplace_back(0x44444444);
    gbtBuffer.emplace_back(0x66666666);
    // gbt.moveToBuffer(gbtBuffer);
    auto rdh = createRDH(mId, orbit, bunchCrossing, sizeof(gbtBuffer));
    dumpRDH(rdh);
    size_t index = mBuffer.size();
    std::cout << fmt::format("index={} rdh size {} gbtbuffer elements {}\n",
                             index, sizeof(rdh), gbtBuffer.size());
    mBuffer.resize(mBuffer.size() + sizeof(rdh) / 4 + gbtBuffer.size());
    memcpy(&mBuffer[index], &rdh, sizeof(rdh));
    memcpy(&mBuffer[index] + sizeof(rdh), &gbtBuffer[0], gbtBuffer.size() * sizeof(gbtBuffer[0]));
    break;
  }
  std::cout << fmt::format("gbts2buffer : orbit {} bx {} buffer size {} => {}\n",
                           orbit, bunchCrossing, prev, mBuffer.size());
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
  std::cout << fmt::format("CRUEncoder({}) buffer size {}\n", mId, mBuffer.size());

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
