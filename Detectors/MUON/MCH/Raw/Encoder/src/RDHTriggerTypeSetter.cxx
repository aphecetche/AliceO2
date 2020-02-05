// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "RDHTriggerTypeSetter.h"
#include "DetectorsRaw/HBFUtils.h"
#include <vector>
#include "MCHRawCommon/RDHFields.h"
#include "MCHRawCommon/RDHManip.h"
#include "Headers/RAWDataHeader.h"

namespace o2::mch::raw
{
/// Ensure the triggerType of each RDH is correctly set
template <typename RDH>
void setTriggerType(gsl::span<uint8_t> pages,
                    gsl::span<const InteractionRecord> interactions)
{
  o2::raw::HBFUtils hbfutils(interactions[0]);
  std::vector<o2::InteractionRecord> emptyHBFs;
  hbfutils.fillHBIRvector(emptyHBFs, interactions[0], interactions[interactions.size() - 1]);

  forEachRDH<RDH>(pages, [&hbfutils](RDH& rdh, gsl::span<uint8_t>::size_type) {
    o2::InteractionRecord rec(rdhBunchCrossing(rdh), rdhOrbit(rdh));
    rdhTriggerType(rdh, rdhTriggerType(rdh) | hbfutils.triggerType(rec));
  });
}

template void setTriggerType<o2::header::RAWDataHeaderV4>(gsl::span<uint8_t> pages,
                                                          gsl::span<const InteractionRecord> interactions);
} // namespace o2::mch::raw
