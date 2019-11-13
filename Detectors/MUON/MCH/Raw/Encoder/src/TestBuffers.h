// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_ENCODER_TEST_BUFFERS_H
#define O2_MCH_RAW_ENCODER_TEST_BUFFERS_H

#include <vector>
#include <cstdint>
#include <gsl/span>
#include "MCHRawCommon/RDHManip.h"
#include <fmt/format.h>
#include "MCHRawEncoder/CRUEncoder.h"
#include "MCHRawEncoder/Encoder.h"

namespace o2::mch::raw::impl
{

std::vector<uint8_t> encodePedestalBuffer(CRUEncoder& cru, int elinkId);

template <typename FORMAT, typename CHARGESUM>
std::vector<uint8_t> createPedestalBuffer(int elinkId)
{
  uint8_t cruId(0);

  auto cru = createCRUEncoderNoPhase<FORMAT, CHARGESUM>(cruId);

  return encodePedestalBuffer(*cru, elinkId);
}

template <typename RDH>
std::vector<uint8_t> createTestBuffer(gsl::span<uint8_t> data)
{
  assert(data.size() % 16 == 0);
  std::vector<uint8_t> buffer;
  auto payloadSize = data.size();
  if (payloadSize > (1 << 16) - sizeof(RDH)) {
    throw std::logic_error(fmt::format("cannot generate a buffer with a payload above {} (tried {})", 0xFFFF - sizeof(RDH), payloadSize));
  }
  auto rdh = o2::mch::raw::createRDH<RDH>(0, 0, 1234, 567, payloadSize);
  o2::mch::raw::appendRDH<RDH>(buffer, rdh);
  std::copy(data.begin(), data.end(), std::back_inserter(buffer));
  return buffer;
}
} // namespace o2::mch::raw::impl

#endif
