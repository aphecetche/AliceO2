// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_DCS_DETA_PROCESSOR_SPEC_H
#define O2_DCS_DETA_PROCESSOR_SPEC_H

#include "DetectorsDCS/DataPointCompositeObject.h"
#include "Framework/DeviceSpec.h"
#include "Framework/Task.h"
#include "Framework/Logger.h"
#include <limits>
#include <vector>

namespace
{
void showRanges(gsl::span<const o2::dcs::DataPointCompositeObject> dpcoms)
{
    double minValue = std::numeric_limits<double>::max();
    double maxValue = std::numeric_limits<double>::min();

    for (auto dp: dpcoms) {
        std::string alias = dp.id.get_alias();
        if (alias.find("DETA/TestDouble_")!=std::string::npos) {
            minValue = std::min(minValue,o2::dcs::getValue<double>(dp));
            maxValue = std::max(maxValue,o2::dcs::getValue<double>(dp));
        }
    }
    LOG(INFO) << fmt::format("DetAProcessor: min {:7.2f} max {:7.2f}",minValue,maxValue);
}
} // namespace

namespace o2
{
namespace dcs
{
class DCSDetAProcessor : public o2::framework::Task
{
 public:

  void run(o2::framework::ProcessingContext& pc) final
  {
    auto dpcoms = pc.inputs().get<gsl::span<o2::dcs::DataPointCompositeObject>>("input");
    showRanges(dpcoms);
  }

}; // end class
} // namespace dcs

namespace framework
{

DataProcessorSpec getDCSDetAProcessorSpec()
{
  return DataProcessorSpec{
    "dcs-deta-processor",
    Inputs{{"input", "DETA", "DATAPOINTS"}, {"inputdelta", "DETA", "DATAPOINTSdelta"}},
    Outputs{},
    AlgorithmSpec{adaptFromTask<o2::dcs::DCSDetAProcessor>()},
    Options{}};
} // namespace framework

} // namespace framework
} // namespace o2

#endif
