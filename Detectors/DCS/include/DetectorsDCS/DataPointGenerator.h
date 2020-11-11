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

template <typename T>
std::vector<DataPointCompositeObject> generateRandomDataPoints(const std::vector<std::string>& aliases,
                                                               T min,
                                                               T max,
                                                               const std::string& refDate);

} // namespace o2::dcs

#endif
