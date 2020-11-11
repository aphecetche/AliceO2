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
#include "DetectorsDCS/DataPointCreator.h"
#include "DetectorsDCS/DataPointIdentifier.h"
#include "DetectorsDCS/DataPointValue.h"
#include "DetectorsDCS/DeliveryType.h"
#include "DetectorsDCS/GenericFunctions.h"
#include "Framework/ConfigParamRegistry.h"
#include "Framework/ControlService.h"
#include "Framework/DeviceSpec.h"
#include "Framework/Logger.h"
#include "Framework/Task.h"
#include "Framework/WorkflowSpec.h"
#include <TDatime.h>
#include <TRandom.h>
#include <TStopwatch.h>
#include <ctime>
#include <random>
#include <sstream>
#include <unistd.h>

using namespace o2::framework;

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
  }

  void run(o2::framework::ProcessingContext& pc) final
  {

    uint64_t tfid;
    for (auto& input : pc.inputs()) {
      tfid = header::get<o2::framework::DataProcessingHeader*>(input.header)->startTime;
      LOG(DEBUG) << "tfid = " << tfid;
      if (tfid >= mMaxTF) {
        LOG(INFO) << "Data generator reached TF " << tfid << ", stopping";
        pc.services().get<o2::framework::ControlService>().endOfStream();
        pc.services().get<o2::framework::ControlService>().readyToQuit(o2::framework::QuitRequest::Me);
      }
      break; // we break because one input is enough to get the TF ID
    }

    // here build FBI and/or Delta
    // LOG(DEBUG) << "TF: " << tfid << " --> building binary blob...";
    // uint16_t flags = 0;
    // uint16_t milliseconds = 0;
    // TDatime currentTime;
    // uint32_t seconds = currentTime.Get();

    // here output FBI
    // std::vector<char> buff(mNumDPsFull * sizeof(DPCOM));
    // char* dptr = buff.data();
    // for (int i = 0; i < svect; i++) {
    //   memcpy(dptr + i * sizeof(DPCOM), &dpcomVectFull[i], sizeof(DPCOM));
    // }
    // auto sbuff = buff.size();
    // pc.outputs().snapshot(Output{"DCS", "DATAPOINTS", 0, Lifetime::Timeframe}, buff.data(), sbuff);
    //
    // // here output Delta map
    // std::vector<char> buffDelta(mNumDPsDelta * sizeof(DPCOM));
    // char* dptrDelta = buffDelta.data();
    // for (int i = 0; i < svectDelta; i++) {
    //   memcpy(dptrDelta + i * sizeof(DPCOM), &dpcomVectDelta[i], sizeof(DPCOM));
    // }
    // auto sbuffDelta = buffDelta.size();
    // pc.outputs().snapshot(Output{"DCS", "DATAPOINTSdelta", 0, Lifetime::Timeframe}, buffDelta.data(), sbuffDelta);

    mFirstTF = false;
    mTFs++;
  }

  void endOfStream(o2::framework::EndOfStreamContext& ec) final
  {
  }

 private:
  uint64_t mMaxTF = 1;

  // DeliveryType mtypechar = RAW_CHAR;
  // DeliveryType mtypeint = RAW_INT;
  // DeliveryType mtypedouble = RAW_DOUBLE;
  // DeliveryType mtypestring = RAW_STRING;

  bool mFirstTF = true;
  uint64_t mTFs = 0;
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
