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

#include "DetectorsDCS/DataPointCompositeObject.h"
#include "Framework/Task.h"
#include "Framework/DeviceSpec.h"
#include "Framework/Logger.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <boost/algorithm/string.hpp>

namespace o2
{
namespace dcs
{

using DPCOM = o2::dcs::DataPointCompositeObject;

class DCSDataDispatcher : public o2::framework::Task
{
 public:
  DCSDataDispatcher(std::vector<std::string> subsystems) : mSubsystems(subsystems) {}

  void process(o2::framework::ProcessingContext& pc, bool delta)
  {
    auto inputName = fmt::format("input{}", (delta ? "delta" : ""));
    auto dpcoms = pc.inputs().get<gsl::span<o2::dcs::DataPointCompositeObject>>(inputName.c_str());

    // build a map split by first 3 characters of the DataPointIdentifier
    std::unordered_map<std::string, std::vector<DPCOM>> mapPerSubsystem;

    for (auto o : mSubsystems) {
      auto upperSubsystem = boost::to_upper_copy(o);
      mapPerSubsystem.emplace(o, std::vector<DPCOM>{});
      for (auto dp : dpcoms) {
        std::string alias = dp.id.get_alias();
        if (boost::to_upper_copy(alias.substr(0, o.size())) == upperSubsystem) {
          mapPerSubsystem[o].push_back(dp);
        }
      }
    }

    for (const auto& [sub, dpcoms] : mapPerSubsystem) {
      o2::header::DataOrigin origin;
      origin.runtimeInit(sub.c_str());
      o2::header::DataDescription desc;
      desc.runtimeInit(fmt::format("DATAPOINTS{}", (delta ? "delta" : "")).c_str());
      // we _always_ output for each subsystem (even empty ones)
      pc.outputs().snapshot(framework::Output{origin, desc, 0, framework::Lifetime::Timeframe}, dpcoms);
    }
  }

  void run(o2::framework::ProcessingContext& pc) final
  {
    process(pc, false);
    process(pc, true);
  }

 private:
  std::vector<std::string> mSubsystems;
}; // end class
} // namespace dcs

namespace framework
{
DataProcessorSpec getDCSDataDispatcherSpec()
{
  std::vector<std::string> dets = {"DETA", "DETB"};

  std::vector<OutputSpec> outputs;
  for (auto d : dets) {
    o2::header::DataOrigin origin;
    origin.runtimeInit(d.c_str());
    outputs.emplace_back(origin, "DATAPOINTS", 0, Lifetime::Timeframe);
    outputs.emplace_back(origin, "DATAPOINTSdelta", 0, Lifetime::Timeframe);
  }

  return DataProcessorSpec{
    "dcs-data-dispatcher",
    Inputs{{"input", "DCS", "DATAPOINTS"}, {"inputdelta", "DCS", "DATAPOINTSdelta"}},
    outputs,
    AlgorithmSpec{adaptFromTask<o2::dcs::DCSDataDispatcher>(dets)},
    Options{
      {"max-cycles-no-full-map", VariantType::Int64, 6000ll, {"max num of cycles between the sending of 2 full maps"}}}};
}
} // namespace framework
} // namespace o2

#endif
