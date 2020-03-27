// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHRawEncoder/FeeLinkSetter.h"
#include "MCHRawCommon/RDHManip.h"
#include "Headers/RAWDataHeader.h"

namespace o2::mch::raw
{
template <typename RDH>
void assignFeeLink(gsl::span<std::byte> buffer,
                   std::function<std::optional<FeeLinkId>(uint16_t solarId)> solar2fee)
{
  throw std::logic_error("Re-implement me or trash me ?");
  // forEachRDH<RDH>(buffer, [&solar2cru](RDH& rdh, gsl::span<std::byte>::size_type) {
  //   // update the (cru,link) from the feeId
  //   uint16_t solarId = rdhFeeId(rdh);
  //   auto opt = solar2cru(solarId);
  //   if (!opt.has_value()) {
  //     std::cout << "ERROR : no (cru,link) mapping for solar " << solarId << "\n";
  //   }
  //   rdhFeeId(rdh, opt.value().feeId());
  //   rdhLinkId(rdh, opt.value().linkId());
  // });
}

template void assignFeeLink<o2::header::RAWDataHeaderV4>(gsl::span<std::byte> buffer,
                                                         std::function<std::optional<FeeLinkId>(uint16_t solarId)> solar2cru);

} // namespace o2::mch::raw
