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

using namespace o2::mch::raw;

CRUEncoder::CRUEncoder(uint8_t cruId) : mId(cruId), mOrbit{}, mBunchCounter{}, mRDH{}, mBuffer(1024), mGBTs{::makeArray<24>([](size_t i) { return GBTEncoder(i); })}
{
}

void CRUEncoder::addChannelChargeSum(uint32_t bx, uint8_t solarId, uint8_t elinkId, uint8_t chId, uint16_t timestamp, uint32_t chargeSum)
{
  auto& gbt = mGBTs[solarId];
  gbt.addChannelChargeSum(bx, elinkId, chId, timestamp, chargeSum);
}

void CRUEncoder::finalize()
{
  // here should :
  // - add padding so all GBTs have the same amount of data words
  // - split data into 8KB chunks, each with its own RDH
  // - call a callback with each 8KB page ? (so it can e.g. be written to disk) ?

  // find the GBT which has the more bits
  auto e = std::max_element(begin(mGBTs), end(mGBTs),
                            [](const GBTEncoder& a, const GBTEncoder& b) {
                              return a.maxLen() < b.maxLen();
                            });
  int alignToSize = e->maxLen();

  for (auto& gbt : mGBTs) {
    gbt.fillWithSync(alignToSize);
  }
}

void CRUEncoder::printStatus()
{
  std::cout << fmt::format("CRUEncoder({}) buffer size {}", mId, mBuffer.size()) << "\n";
  for (auto& gbt : mGBTs) {
    gbt.printStatus();
  }
}
