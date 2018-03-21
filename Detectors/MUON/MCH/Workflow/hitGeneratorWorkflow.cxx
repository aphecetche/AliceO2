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
  Segmentation seg(itx.options().get<int>("deid"), true);

  return [seg](of::ProcessingContext& ctx) { generateFakeHits(ctx.allocator(), seg); };
}

of::ConfigParamSpec deIdOption(const char* help) {
  return of::ConfigParamSpec("deid", of::VariantType::Int, 100, {help});
}

of::Inputs noInput{};
of::Outputs noOutput{};

of::DataProcessorSpec hitGeneratorSpec(uint detElemId)
{
  return {
    // clang-format off
      "mch-hit-generator",
      noInput,
      of::Outputs{{"MCH", "HITS", detElemId, of::OutputSpec::Lifetime::Timeframe}},
      of::AlgorithmSpec{initHitGenerator},
      { deIdOption("which detection element to generate hits for") }
    // clang-format on
  };
}

of::DataProcessorSpec preclusterizerSpec(uint detElemId)
{
  return {
    // clang-format off
          "mch-preclusterizer",
          of::Inputs{{"mch-hits", "MCH", "HITS", detElemId, of::InputSpec::Lifetime::Timeframe}},
          noOutput,
          of::AlgorithmSpec{[](of::ProcessingContext &) {
            return;
          }},
          { deIdOption("which detection element to preclusterize") }
    // clang-format on
  };
}

of::WorkflowSpec hitGeneratorWorkflow()
{
  return {
    // clang-format off
      hitGeneratorSpec(100),
      preclusterizerSpec(100)
    // clang-format on
  };
}

void defineDataProcessing(of::WorkflowSpec& specs)
{
  of::WorkflowSpec workflow{ hitGeneratorWorkflow() };

  specs.swap(workflow);
}
