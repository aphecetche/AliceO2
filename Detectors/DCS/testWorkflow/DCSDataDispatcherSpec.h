// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_DCS_DATA_DISPATCHER_V2_H
#define O2_DCS_DATA_DISPATCHER_V2_H

/// @file   DCSDataDispatcherSpec
/// @brief  Prototype of DCS Data Dispatcher
///
/// Takes as input a Full Buffer Image (or delta) and
/// splits it per subsystem (using the first three letters
/// of the DataPointIdentifier alias)

#include <unistd.h>
#include <TRandom.h>
#include <TStopwatch.h>
#include "DetectorsDCS/DataPointIdentifier.h"
#include "DetectorsDCS/DataPointCompositeObject.h"
#include "DetectorsDCS/DeliveryType.h"
#include "DetectorsDCS/DCSProcessor.h"
#include "DetectorsCalibration/Utils.h"
#include "CCDB/CcdbApi.h"
#include "Framework/DeviceSpec.h"
#include "Framework/ConfigParamRegistry.h"
#include "Framework/ControlService.h"
#include "Framework/WorkflowSpec.h"
#include "Framework/Task.h"
#include "Framework/Logger.h"
#include <cctype>
#include <gsl/span>
#include <boost/algorithm/string.hpp>
#include <unordered_map>

namespace o2
{
namespace dcs
{

using namespace o2::dcs;
using DPID = o2::dcs::DataPointIdentifier;
using DPCOM = o2::dcs::DataPointCompositeObject;

class DCSDataDispatcher : public o2::framework::Task
{
 public:
  DCSDataDispatcher(std::vector<std::string> subsystems) : mSubsystems(subsystems) {}

  void init(o2::framework::InitContext& ic) final
  {
  }

  void run(o2::framework::ProcessingContext& pc) final
  {
    // build a map split by first 3 characters of the DataPointIdentifier

    //auto rawcharDelta = pc.inputs().get<const char*>("inputDelta");

    // full map
    auto dpcoms = pc.inputs().get<gsl::span<o2::dcs::DataPointCompositeObject>>("input");
    LOG(INFO) << "dpcoms.size()=" << dpcoms.size() << "\n";

    std::unordered_map<std::string, std::vector<DPCOM>> mapPerSubsystem;

    for (auto o : mSubsystems) {
      auto upperSubsystem = boost::to_upper_copy(o);
      for (auto dp : dpcoms) {
        std::string alias = dp.id.get_alias();
        if (boost::to_upper_copy(alias.substr(0, o.size())) == upperSubsystem) {
          std::cout << o << " : " << alias << "\n";
        }
      }
    }

    // send full map per subsystem
  }

  void endOfStream(o2::framework::EndOfStreamContext& ec) final
  {
  }

 private:
  std::vector<std::string> mSubsystems;
}; // end class
} // namespace dcs

namespace framework
{

DataProcessorSpec getDCSDataDispatcherSpec()
{
  std::vector<std::string> dets = {"TEST", "MCH"};

  std::vector<OutputSpec> outputs;
  for (auto d : dets) {
    o2::header::DataOrigin origin;
    origin.runtimeInit(d.c_str());
    outputs.emplace_back(origin, "DATAPOINTS", 0, Lifetime::Timeframe);
    outputs.emplace_back(origin, "DATAPOINTSdelta", 0, Lifetime::Timeframe);
  }

  return DataProcessorSpec{
    "dcs-data-dispatcher",
    Inputs{{"input", "DCS", "DATAPOINTS"}, {"inputDelta", "DCS", "DATAPOINTSdelta"}},
    outputs,
    AlgorithmSpec{adaptFromTask<o2::dcs::DCSDataDispatcher>(dets)},
    Options{
      {"max-cycles-no-full-map", VariantType::Int64, 6000ll, {"max num of cycles between the sending of 2 full maps"}},
      {"process-full-delta-map", VariantType::Bool, false, {"to process the delta map as a whole instead of per DP"}}}};
} // namespace framework

} // namespace framework
} // namespace o2

#endif
