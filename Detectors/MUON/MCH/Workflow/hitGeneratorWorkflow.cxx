// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See https://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

///
/// @author  Laurent Aphecetche

#include "hitGeneratorWorkflow.h"

#include "Framework/runDataProcessing.h"
#include "Framework/DataAllocator.h"
#include "Framework/AlgorithmSpec.h"
#include "Framework/ProcessingContext.h"
#include "Framework/InitContext.h"
#include "Framework/DataProcessorSpec.h"
#include "MCHMappingInterface/Segmentation.h"
#include <iostream>
#include <chrono>
#include "Framework/OutputSpec.h"

using o2::mch::mapping::Segmentation;
using namespace std::chrono_literals;
namespace of = o2::framework;

void generateFakeHits(of::DataAllocator& da, const Segmentation& seg)
{
  LOG(DEBUG) << "would make hits here";
  LOG(INFO) << seg.nofPads();
  std::this_thread::sleep_for(1s);
}

of::AlgorithmSpec::ProcessCallback initHitGenerator(of::InitContext& itx)
{
  Segmentation seg(100, true);

  LOG(INFO) << seg.nofPads();

  return [seg](of::ProcessingContext& ctx) { generateFakeHits(ctx.allocator(), seg); };
}

of::WorkflowSpec hitGeneratorWorkflow()
{
  return {
    // clang-format off
      {
          "mch-hit-generator",
          of::Inputs{},
          of::Outputs{ {"MCH","HITS",100,of::OutputSpec::Lifetime::Timeframe}},
          of::AlgorithmSpec{initHitGenerator},
      {
          "mch-preclusterizer",
          of::Inputs{{"mch-hits","MCH","HITS",100,of::InputSpec::Lifetime::Timeframe}},
          of::Outputs{},
          of::AlgorithmSpec{[](of::ProcessingContext&) {
            return;
          }}
      }
  }; // clang-format on
}

void defineDataProcessing(of::WorkflowSpec& specs)
{
  of::WorkflowSpec workflow{ hitGeneratorWorkflow() };

  specs.swap(workflow);
}
