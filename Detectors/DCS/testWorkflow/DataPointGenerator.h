// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_DCS_DATAPOINT_GENERATOR_H
#define O2_DCS_DATAPOINT_GENERATOR_H

#include "DetectorsDCS/DeliveryType.h"
#include "DetectorsDCS/DataPointCompositeObject.h"
#include <vector>

namespace o2::dcs
{

struct DataPointHint {
  std::string alias;
  o2::dcs::DeliveryType type;
  uint64_t min;
  uint64_t max;
};

std::vector<o2::dcs::DataPointCompositeObject>
  generateRandomFBI(const std::vector<DataPointHint>& dphints);
std::vector<o2::dcs::DataPointCompositeObject> generateRandomDelta(const std::vector<DataPointHint>& dphints);

std::vector<std::string> expandAliases(const std::vector<std::string>& patternedAliases);

} // namespace o2::dcs

#endif
