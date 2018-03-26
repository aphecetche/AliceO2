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
#include <Headers/DataHeader.h>
#include "Framework/OutputSpec.h"
#include "generateHitPositions.h"
using o2::mch::mapping::Segmentation;
using namespace std::chrono_literals;
namespace of = o2::framework;

struct FakeHit {
  double x;
  double y;
  int charge;
  friend std::ostream& operator<<(std::ostream& os, const FakeHit& hit)
  {
    os << "x: " << hit.x << " y: " << hit.y << " charge: " << hit.charge;
    return os;
  }
};

void generateFakeHits(gsl::span<FakeHit>& hits, const Segmentation& seg)
{
  LOG(ERROR) << "HERE AS WELL";
  auto hitPositions = o2::mch::test::generateHitPositions(hits.size(), seg);

  for (auto i = 0; i < hitPositions.size(); ++i) {
    auto& fh = hits[i];
    auto& hp = hitPositions[i];
    fh.x = hp.x;
    fh.y = hp.y;
    fh.charge = 10;
    LOG(INFO) << fh;
  }
}

of::AlgorithmSpec::ProcessCallback initHitGenerator(of::InitContext& itx)
{
  auto detElemId = itx.options().get<int>("deid");

  Segmentation seg(detElemId, true);

  auto nhits = itx.options().get<int>("nofhitperde");

  LOG(WARNING) << "detElemId=" << detElemId << " nhits=" << nhits;

  return [seg, nhits, detElemId](of::ProcessingContext& ctx) {
    auto outputHits = ctx.allocator().make<FakeHit>(of::OutputSpec{ "MCH", "HITS", static_cast<o2::header::DataHeader::SubSpecificationType>(detElemId) }, static_cast<size_t>(nhits));
    //gsl::span<FakeHit> outputHits;
    LOG(ERROR) << "Calling generateFakeHits with " << outputHits.size() << " hits / " << nhits;
    generateFakeHits(outputHits, seg);
    sleep(1);
  };
}

of::ConfigParamSpec deIdOption(const char* help)
{
  return of::ConfigParamSpec("deid", of::VariantType::Int, 100, { help });
}

of::ConfigParamSpec nofHitPerDEOption(const char* help)
{
  return of::ConfigParamSpec("nofhitperde", of::VariantType::Int, 10, { help });
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
      { deIdOption("which detection element to generate hits for"),
        nofHitPerDEOption("number of hits to generate per detection element")
      }
    // clang-format on
  };
}

of::DataProcessorSpec hitDumper(uint detElemId)
{
  return {
    // clang-format off
      "mch-hit-dumper",
      of::Inputs{{"mch-hits", "MCH", "HITS", detElemId, of::InputSpec::Lifetime::Timeframe}},
      noOutput,
      of::AlgorithmSpec{[](of::ProcessingContext& ctx) {
        LOG(INFO) << "hit dump";
        auto hits = ctx.inputs().get("hits");
        LOG(INFO) << hits.header;
      }},
      { deIdOption("which detection element to dump hit for") }
    // clang-format on
  };
}

of::DataProcessorSpec digitizer(uint detElemId)
{
  return {
    // clang-format off
      "mch-digitizer",
      of::Inputs{{"mch-hits", "MCH", "HITS", detElemId, of::InputSpec::Lifetime::Timeframe}},
      of::Outputs{{"MCH","DIGITS",detElemId, of::OutputSpec::Lifetime::Timeframe}},
      of::AlgorithmSpec{[](of::ProcessingContext &) {
        return;
      }},
      { deIdOption("which detection element to preclusterize") }
    // clang-format on
  };
}

of::DataProcessorSpec preclusterizerSpec(uint detElemId)
{
  return {
    // clang-format off
      "mch-preclusterizer",
      of::Inputs{{"mch-digits", "MCH", "DIGITS", detElemId, of::InputSpec::Lifetime::Timeframe}},
      noOutput,
      of::AlgorithmSpec{[](of::ProcessingContext &) {
        return;
      }},
      { deIdOption("which detection element to preclusterize") }
    // clang-format on
  };
}

of::WorkflowSpec hitGeneratorWorkflow(uint detElemId)
{
  return {
    // clang-format off
      hitGeneratorSpec(detElemId),
      hitDumper(detElemId)
      // digitizer(detElemId),
      // preclusterizerSpec(detElemId)
    // clang-format on
  };
}

void defineDataProcessing(of::WorkflowSpec& specs)
{
  of::WorkflowSpec workflow{ hitGeneratorWorkflow(100) };

  specs.swap(workflow);
}
