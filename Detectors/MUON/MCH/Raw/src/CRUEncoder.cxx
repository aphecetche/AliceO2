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
#include "MakeArray.h"
#include <fmt/format.h>
#include "NofBits.h"
#include <algorithm>

using namespace o2::mch::raw;

CRUEncoder::CRUEncoder(uint8_t cruId) : mId(cruId), mRDH{}, mBuffer(1024), mGBTs{::makeArray<24>([](size_t i) { return GBTEncoder(i); })}, mTriggerInfoChanges{}
{
  assertNofBits("cruId", cruId, 12);
  mRDH.cruId = cruId;
  mRDH.dpwId = 0; // FIXME: fill this ?
}

void CRUEncoder::addChannelChargeSum(uint32_t bx, uint8_t solarId, uint8_t elinkId, uint8_t chId, uint16_t timestamp, uint32_t chargeSum)
{
  mGBTs[solarId].addChannelChargeSum(bx, elinkId, chId, timestamp, chargeSum);
}

void CRUEncoder::addOrbitBC(uint32_t orbit, uint16_t bunchCrossing)
{
  if (len()) {
    align();
  }
  mTriggerInfoChanges.push_back({len(), orbit, bunchCrossing});
}

void CRUEncoder::addRDH(int len,
                        uint8_t linkId,
                        uint16_t pagesCounter,
                        bool lastPage)
{
  constexpr int PAGE_SIZE = 8192;

  mRDH.feeId = 0; //FIXME: what is this field supposed to contain ? unclear to me.
  mRDH.priorityBit = 0;
  mRDH.blockLength = PAGE_SIZE - mRDH.headerSize; // FIXME: the blockLength disappears in RDHv5 ?
  mRDH.offsetNextPacket = PAGE_SIZE;
  mRDH.memorySize = 0;    //FIXME: fill this ?
  mRDH.packetCounter = 0; // FIXME: fill this ?
  mRDH.triggerType = 0;   // FIXME: fill this ?
  mRDH.detectorField = 0; // FIXME: fill this ?
  mRDH.par = 0;           // FIXME: fill this ?
  mRDH.stopBit = lastPage;
  mRDH.pagesCounter = pagesCounter;

  auto it = std::lower_bound(begin(mTriggerInfoChanges), end(mTriggerInfoChanges), TriggerInfoChange{len, 0, 0});
  auto orbit = it->orbit;
  auto bunchCrossing = it->bc;

  mRDH.triggerOrbit = orbit;
  mRDH.heartbeatOrbit = orbit; // FIXME: RDHv5 has only triggerOrbit ?
  mRDH.triggerBC = bunchCrossing;
  mRDH.heartbeatBC = bunchCrossing; // FIXME: RDHv5 has only triggerBC ?
}

bool CRUEncoder::areGBTsAligned() const
{
  auto len = mGBTs[0].len();
  for (int i = 1; i < mGBTs.size(); i++) {
    if (mGBTs[i].len() != len) {
      return false;
    }
  }
  return true;
}

void CRUEncoder::align()
{
  int l = len();
  for (auto& gbt : mGBTs) {
    gbt.align(l);
  }
}

const GBTEncoder& CRUEncoder::maxElement() const
{
  auto e = std::max_element(begin(mGBTs), end(mGBTs),
                            [](const GBTEncoder& a, const GBTEncoder& b) {
                              return a.len() < b.len();
                            });
  return *e;
}

int CRUEncoder::phase() const
{
  // return the phase of the widest GBT
  if (len()) {
    return maxElement().phase();
  }
  return -1;
}

int CRUEncoder::len() const
{
  // find the GBT which has the more bits
  return maxElement().len();
}

void CRUEncoder::finalize()
{
  // here should :
  // - add padding so all GBTs have the same amount of data words (i.e. "align" them)
  // - split data into 8KB chunks, each with its own RDH
  // - call a callback with each 8KB page ? (so it can e.g. be written to disk) ?

  align();
}

void CRUEncoder::printStatus() const
{
  std::cout << fmt::format("CRUEncoder({}) buffer size {} gbts are{}aligned len {} phase {}", mId, mBuffer.size(), (areGBTsAligned() ? " " : " not "), len(), phase()) << "\n";

  if (mTriggerInfoChanges.size()) {
    for (int i = 0; i < mTriggerInfoChanges.size(); i++) {
      std::cout << mTriggerInfoChanges[i] << "\n";
    }
  }
  //for (auto& gbt : mGBTs) {
  for (int i = 0; i < 2; i++) {
    const auto& gbt = mGBTs[i];
    gbt.printStatus();
  }
}
