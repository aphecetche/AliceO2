// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction. 

#ifndef O2_DCS_PROCESSOR_DEVICE_H
#define O2_DCS_PROCESSOR_DEVICE_H

#include "Framework/Task.h"
#include <gsl/span>
#include "DetectorsDCS/DataPointCompositeObject.h"

namespace o2::dcs {
class DCSProcessorDevice : public o2::framework::Task
{
 public:
  auto getData(o2::framework::ProcessingContext& pc)
  {
    using DPCOMS = gsl::span<o2::dcs::DataPointCompositeObject>;
    auto fbi = pc.inputs().get<DPCOMS>("input");
    auto delta = pc.inputs().get<DPCOMS>("inputdelta");
    return std::make_pair(fbi, delta);
  }
};

}

#endif

