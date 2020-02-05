// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_ENCODER_DIGIT_ENCODER_H
#define O2_MCH_RAW_ENCODER_DIGIT_ENCODER_H

#include <gsl/span>
#include "MCHRawEncoder/Encoder.h"
#include <functional>
#include <memory>
#include <cstdint>
#include "MCHRawElecMap/DsDetId.h"
#include "MCHMappingFactory/CreateSegmentation.h"
#include "MCHBase/Digit.h"
#include <fmt/printf.h>

namespace o2::mch::raw
{

template <typename CHARGESUM>
void encodeDigits(gsl::span<o2::mch::Digit> digits,
                  std::unique_ptr<Encoder>& encoder,
                  std::function<std::optional<DsElecId>(DsDetId)> det2elec,
                  uint32_t orbit,
                  uint16_t bc)

{
  static int nadd{0};

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
    int dschid = mapping::segmentation(deid).padDualSampaChannel(d.getPadID());
    uint16_t ts(static_cast<int>(d.getTimeStamp()) & 0x3FF);
    int sampachid = dschid % 32;

    auto clusters = createSampaClusters<CHARGESUM>(ts, d.getADC());

    fmt::printf(
      "nadd %6d deid %4d dsid %4d dschid %2d "
      "solarId %3d groupId %2d elinkId %2d sampach %2d : %s\n",
      nadd++, deid, dsid, dschid,
      elecId.solarId(), elecId.elinkGroupId(), elecId.elinkId(), sampachid,
      asString(clusters[0]).c_str());

    encoder->addChannelData(elecId, sampachid, clusters);
  }
}
} // namespace o2::mch::raw
#endif
