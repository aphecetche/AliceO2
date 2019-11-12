// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "TestBuffers.h"
#include "MCHRawEncoder/Encoder.h"
#include "MCHRawCommon/SampaCluster.h"
#include "MCHRawCommon/DataFormats.h"

using namespace o2::mch::raw;
using namespace o2::mch::raw::dataformat;

std::vector<uint8_t> encodePedestalBuffer(CRUEncoder& cru, int elinkId)
{
  uint32_t bx(0);
  uint8_t solarId(0);
  uint16_t ts(0);
  uint32_t orbit{42};
  const int N{1};

  for (int i = 0; i < N; i++) {
    cru.startHeartbeatFrame(orbit, bx + i);
    for (uint16_t j = 0; j < 31; j++) {
      std::vector<uint16_t> samples;
      for (auto k = 0; k < j + 1; k++) {
        samples.push_back(10 + j);
      }
      cru.addChannelData(solarId, elinkId, j, {SampaCluster(ts, samples)});
    }
  }
  std::vector<uint8_t> buffer;
  cru.moveToBuffer(buffer);
  return buffer;
}

std::vector<uint8_t> createBarePedestalBuffer(int elinkId)
{
  uint8_t cruId(0);

  auto cru = createCRUEncoderNoPhase<Bare, SampleMode>(cruId);

  return encodePedestalBuffer(*cru, elinkId);
}

std::vector<uint8_t> createUserLogicPedestalBuffer(int elinkId)
{
  uint8_t cruId(0);

  auto cru = createCRUEncoder<UserLogic, SampleMode>(cruId);

  return encodePedestalBuffer(*cru, elinkId);
}
