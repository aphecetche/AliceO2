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
#include <map>

#include "MCHRaw/GBTEncoder.h"
#include <iostream>
#include <fmt/format.h>

namespace o2::mch::raw
{

class CRUEncoder
{

 public:
  CRUEncoder(uint16_t cruId);

  void addChannelChargeSum(uint8_t solarId, uint8_t elinkId, uint8_t chId, uint16_t timestamp, uint32_t chargeSum);

  void printStatus(int maxgbt = -1) const;

  // sets the trigger (orbit,bunchCrossing) to be used
  // for all generated RDHs (until next call to this method).
  // causes the alignment of the underlying gbts.
  void startHeartbeatFrame(uint32_t orbit, uint16_t bunchCrossing);

  size_t moveToBuffer(std::vector<uint32_t>& buffer);

 private:
  void closeHeartbeatFrame(uint32_t orbit, uint16_t bunchCrossing);
  void gbts2buffer(uint32_t orbit, uint16_t bunchCrossing);

 private:
  uint16_t mId;
  uint32_t mOrbit;
  uint16_t mBunchCrossing;
  std::vector<uint32_t> mBuffer;
  std::array<GBTEncoder, 24> mGBTs;
};

} // namespace o2::mch::raw

#endif
