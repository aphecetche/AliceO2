// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHBase/Digit.h"
#include "MCHMappingFactory/CreateSegmentation.h"
#include "MCHRawCommon/DataFormats.h"
#include "MCHRawElecMap/DsDetId.h"
#include "MCHRawEncoder/DigitEncoder.h"
#include "MCHRawEncoder/Encoder.h"
#include <cstdint>
#include <fmt/printf.h>
#include <functional>
#include <gsl/span>
#include <memory>

namespace o2::mch::raw
{
template <typename FORMAT, typename CHARGESUM>
DigitEncoder createDigitEncoder(std::function<std::optional<DsElecId>(DsDetId)> det2elec)
{
  auto encoder = createEncoder<FORMAT, CHARGESUM>().release();
  return [encoder, det2elec](gsl::span<o2::mch::Digit> digits,
                             std::vector<uint8_t>& buffer,
                             uint32_t orbit,
                             uint16_t bc) {
    static int nadd{0};
    encoder->startHeartbeatFrame(orbit, bc);

    for (auto d : digits) {
      int deid = d.getDetID();
      int dsid = mapping::segmentation(deid).padDualSampaId(d.getPadID());
      DsDetId detId{deid, dsid};
      auto dselocopt = det2elec(DsDetId(deid, dsid));
      if (!dselocopt.has_value()) {
        std::cout << fmt::format("WARNING : got no location for (de,ds)=({},{})\n", deid, dsid);
        continue;
      }
      DsElecId elecId = dselocopt.value();
      if (elecId.solarId() != 96) {
        continue;
      }
      int dschid = mapping::segmentation(deid).padDualSampaChannel(d.getPadID());
      uint16_t ts(static_cast<int>(d.getTimeStamp()) & 0x3FF); // FIXME: add a warning if timestamp is not in range
      int sampachid = dschid % 32;

      auto clusters = createSampaClusters<CHARGESUM>(ts, d.getADC() & 0xFFFFF); // FIXME: add a warning if ADC is not in range

      fmt::printf(
        "nadd %6d deid %4d dsid %4d dschid %2d "
        "solarId %3d groupId %2d elinkId %2d sampach %2d : %s\n",
        nadd++, deid, dsid, dschid,
        elecId.solarId(), elecId.elinkGroupId(), elecId.elinkId(), sampachid,
        asString(clusters[0]).c_str());

      encoder->addChannelData(elecId, sampachid, clusters);
    }
    encoder->moveToBuffer(buffer);
  };
}

DigitEncoder createDigitEncoder(bool userLogic, std::function<std::optional<DsElecId>(DsDetId)> det2elec)
{
  if (userLogic) {
    return createDigitEncoder<UserLogicFormat, ChargeSumMode>(det2elec);
  }
  return createDigitEncoder<BareFormat, ChargeSumMode>(det2elec);
}
} // namespace o2::mch::raw
