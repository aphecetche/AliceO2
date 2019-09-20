// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHRawEncoder/CruLinkSetter.h"
#include "MCHRawCommon/RDHManip.h"
#include "Headers/RAWDataHeader.h"

namespace o2::mch::raw
{
template <typename RDH>
void assignCruLink(gsl::span<uint8_t> buffer,
                   std::function<std::optional<CruLinkId>(uint16_t solarId)> solar2cru)
{
  forEachRDH<RDH>(buffer, [&solar2cru](RDH& rdh, gsl::span<uint8_t>::size_type) {
    // update the (cru,link) from the feeId
    uint16_t solarId = rdhFeeId(rdh);
    auto opt = solar2cru(solarId);
    if (!opt.has_value()) {
      std::cout << "ERROR : no (cru,link) mapping for solar " << solarId << "\n";
    }
    rdhCruId(rdh, opt.value().cruId());
    rdhLinkId(rdh, opt.value().linkId());
  });
}

template void assignCruLink<o2::header::RAWDataHeaderV4>(gsl::span<uint8_t> buffer,
                                                         std::function<std::optional<CruLinkId>(uint16_t solarId)> solar2cru);

} // namespace o2::mch::raw
