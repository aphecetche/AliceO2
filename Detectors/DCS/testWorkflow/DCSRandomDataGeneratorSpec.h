// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_DCS_RANDOM_DATA_GENERATOR_SPEC_H
#define O2_DCS_RANDOM_DATA_GENERATOR_SPEC_H

#include "DetectorsDCS/DataPointCompositeObject.h"
#include "DetectorsDCS/DataPointIdentifier.h"
#include "DetectorsDCS/DataPointValue.h"
#include "DetectorsDCS/DataPointGenerator.h"
#include "DetectorsDCS/DeliveryType.h"
#include "DetectorsDCS/GenericFunctions.h"
#include "Framework/ConfigParamRegistry.h"
#include "Framework/ControlService.h"
#include "Framework/DeviceSpec.h"
#include "Framework/Logger.h"
#include "Framework/Task.h"
#include "Framework/WorkflowSpec.h"
#include <ctime>
#include <random>
#include <sstream>
#include <variant>

using namespace o2::framework;

namespace
{
template <typename T>
struct DataPointHint {
  std::string alias;
  T minValue;
  T maxValue;
};

using HintType = std::variant<DataPointHint<double>,
                              DataPointHint<uint32_t>,
                              DataPointHint<int32_t>,
                              DataPointHint<char>,
                              DataPointHint<bool>>;

std::vector<int> generateIntegers(size_t size, int min, int max)
{
  std::uniform_int_distribution<int> distribution(min, max);
  std::mt19937 generator;
  std::vector<int> data(size);
  std::generate(data.begin(), data.end(), [&]() { return distribution(generator); });
  return data;
}

std::vector<o2::dcs::DataPointCompositeObject> generate(const std::vector<HintType> hints,
                                                        float fraction = 1.0)
{
  std::vector<o2::dcs::DataPointCompositeObject> dataPoints;

  auto GenerateVisitor = [](const auto& t) {
    return o2::dcs::generateRandomDataPoints({t.alias}, t.minValue, t.maxValue);
  };

  for (const auto& hint : hints) {
    auto dpcoms = std::visit(GenerateVisitor, hint);
    for (auto dp : dpcoms) {
      dataPoints.push_back(dp);
    }
  }
  if (fraction < 1.0) {
    auto indices = generateIntegers(fraction * dataPoints.size(), 0, dataPoints.size() - 1);
    auto tmp = dataPoints;
    dataPoints.clear();
    for (auto i : indices) {
      dataPoints.push_back(tmp[i]);
    }
  }
  return dataPoints;
}

} // namespace

namespace o2
{
namespace dcs
{
class DCSRandomDataGenerator : public o2::framework::Task
{

  using DPID = o2::dcs::DataPointIdentifier;
  using DPVAL = o2::dcs::DataPointValue;
  using DPCOM = o2::dcs::DataPointCompositeObject;

 public:
  void init(o2::framework::InitContext& ic) final
  {
    mMaxTF = ic.options().get<int64_t>("max-timeframes");

    // here create the list of DataPointHints to be used by the generator
    mDataPointHints.emplace_back(DataPointHint<char>{"DETA/TestChar_0", 'A', 'z'});
    mDataPointHints.emplace_back(DataPointHint<double>{"DETA/TestDouble_[0..5000]", 0, 1700});
    mDataPointHints.emplace_back(DataPointHint<uint32_t>{"DETB/TestInt_[0..100]", 0, 1234});
    mDataPointHints.emplace_back(DataPointHint<bool>{"DETB/TestBool_[00..03]", 0, 1});
  }

  void run(o2::framework::ProcessingContext& pc) final
  {
    auto input = pc.inputs().begin();
    uint64_t tfid = header::get<o2::framework::DataProcessingHeader*>((*input).header)->startTime;
    if (tfid >= mMaxTF) {
      LOG(INFO) << "Data generator reached TF " << tfid << ", stopping";
      pc.services().get<o2::framework::ControlService>().endOfStream();
      pc.services().get<o2::framework::ControlService>().readyToQuit(o2::framework::QuitRequest::Me);
    }

    auto fbi = generate(mDataPointHints);
    pc.outputs().snapshot(Output{"DCS", "DATAPOINTS", 0, Lifetime::Timeframe}, fbi);

    auto delta = generate(mDataPointHints, mDeltaFraction);
    pc.outputs().snapshot(Output{"DCS", "DATAPOINTSdelta", 0, Lifetime::Timeframe}, delta);

    mTFs++;
  }

  void endOfStream(o2::framework::EndOfStreamContext& ec) final
  {
  }

 private:
  uint64_t mMaxTF = 1;
  uint64_t mTFs = 0;
  float mDeltaFraction = 0.05; // Delta is 5% of FBI
  std::vector<HintType> mDataPointHints;
};

} // namespace dcs

namespace framework
{

DataProcessorSpec getDCSRandomDataGeneratorSpec()
{
  return DataProcessorSpec{
    "dcs-random-data-generator",
    Inputs{},
    Outputs{{{"outputDCS"}, "DCS", "DATAPOINTS"}, {{"outputDCSdelta"}, "DCS", "DATAPOINTSdelta"}},
    AlgorithmSpec{adaptFromTask<o2::dcs::DCSRandomDataGenerator>()},
    Options{{"max-timeframes", VariantType::Int64, 99999999999ll, {"max TimeFrames to generate"}}}};
}

} // namespace framework
} // namespace o2

#endif
