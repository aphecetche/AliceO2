// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_DCS_DETB_PROCESSOR_SPEC_H
#define O2_DCS_DETB_PROCESSOR_SPEC_H

#include "DetectorsDCS/DataPointCompositeObject.h"
#include "Framework/DeviceSpec.h"
#include "Framework/Task.h"
#include "Framework/Logger.h"
#include <limits>
#include <vector>

namespace o2
{
namespace dcs
{
class DCSDetBProcessor : public o2::framework::Task
{
 public:
  void run(o2::framework::ProcessingContext& pc) final
  {
    auto dpcoms = pc.inputs().get<gsl::span<o2::dcs::DataPointCompositeObject>>("input");

    double mean = 0;
    int n = 0;

    for (auto dp : dpcoms) {
      std::string alias = dp.id.get_alias();
      if (alias.find("DETB/TestInt_") != std::string::npos) {
        mean += o2::dcs::getValue<uint32_t>(dp);
        ++n;
      }
    }


    if (n>0) {
        LOG(INFO) << fmt::format("DetBProcessor :: mean = {:7.2f}",mean/n);
    }
    else {
        LOG(INFO) << "DetBProcessor: no data";
    }
  }

}; // end class
} // namespace dcs

namespace framework
{

DataProcessorSpec getDCSDetBProcessorSpec()
{
  return DataProcessorSpec{
    "dcs-detb-processor",
    Inputs{{"input", "DETB", "DATAPOINTS"}, {"inputdelta", "DETB", "DATAPOINTSdelta"}},
    Outputs{},
    AlgorithmSpec{adaptFromTask<o2::dcs::DCSDetBProcessor>()},
    Options{}};
} // namespace framework

} // namespace framework
} // namespace o2

#endif
