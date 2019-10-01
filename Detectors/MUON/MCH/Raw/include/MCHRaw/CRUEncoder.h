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

#include "MCHRaw/GBTEncoder.h"
#include "MCHRaw/RAWDataHeader.h" // FIXME: get this from an authoritative source

namespace o2::mch::raw
{

class CRUEncoder
{

 public:
  CRUEncoder(uint8_t cruId);

  void addChannelChargeSum(uint32_t bx, uint8_t solarId, uint8_t elinkId, uint8_t chId, uint16_t timestamp, uint32_t chargeSum);

  void finalize();

  void printStatus();

  // to be used until next call
  // for all generated RDHs
  void addOrbitBC(uint32_t orbit, uint16_t bunchcounter);

 private:
  uint8_t mId;
  uint32_t mOrbit;
  uint16_t mBunchCounter;
  RAWDataHeader mRDH;
  std::vector<uint32_t> mBuffer;
  std::array<GBTEncoder, 24> mGBTs;
};
} // namespace o2::mch::raw

#endif
