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
#include "MCHRawCommon/SampaCluster.h"
#include "MCHRawEncoder/Encoder.h"

namespace o2::mch::raw::impl
{

std::vector<uint8_t> encodePedestalBuffer(Encoder& encoder, uint8_t elinkGroupId)
{
  uint32_t bx(0);
  uint8_t solarId(0);
  uint16_t ts(0);
  uint32_t orbit{42};
  const int N{1};

  uint8_t index{0};

  for (int i = 0; i < N; i++) {
    encoder.startHeartbeatFrame(orbit, bx + i);
    for (uint8_t j = 0; j < 31; j++) {
      std::vector<uint16_t> samples;
      for (auto k = 0; k < j + 1; k++) {
        samples.push_back(10 + j);
      }
      encoder.addChannelData(DsElecId{solarId, elinkGroupId, index}, j, {SampaCluster(ts, samples)});
    }
  }
  std::vector<uint8_t> buffer;
  encoder.moveToBuffer(buffer);
  return buffer;
}
} // namespace o2::mch::raw::impl
