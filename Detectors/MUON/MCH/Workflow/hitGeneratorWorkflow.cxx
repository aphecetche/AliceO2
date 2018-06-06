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

#include "Framework/AlgorithmSpec.h"
#include "Framework/DataAllocator.h"
#include "Framework/DataProcessorSpec.h"
#include "Framework/ExternalFairMQDeviceProxy.h"
#include "Framework/InitContext.h"
#include "Framework/Lifetime.h"
#include "Framework/OutputSpec.h"
#include "Framework/ProcessingContext.h"
#include "MCHMappingInterface/Segmentation.h"
#include "generateHitPositions.h"
#include <Framework/DataProcessingHeader.h>
#include <Headers/DataHeader.h>
#include <chrono>
#include <iostream>

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

struct FakeDigit {
  int dualSampaId;
  int dualSampaChannel;
  friend std::ostream &operator<<(std::ostream &os, const FakeDigit &digit) {
    os << "dualSampaId: " << digit.dualSampaId << " dualSampaChannel: " << digit.dualSampaChannel << " charge: "
       << digit.charge;
    return os;
  }
  int charge;
};

void generateFakeHits(gsl::span<FakeHit> hits, const Segmentation& seg)
{
  auto hitPositions = o2::mch::test::generateHitPositions(hits.size(), seg);

  for (auto i = 0; i < hitPositions.size(); ++i) {
    auto& fh = hits[i];
    auto& hp = hitPositions[i];
    fh.x = hp.x;
    fh.y = hp.y;
    fh.charge = 10;
  }
}

void digitize(const Segmentation& seg, gsl::span<const FakeHit> hits, gsl::span<FakeDigit> digits)
{
  for (auto i = 0; i < hits.size(); ++i) {
    auto& fh = hits[i];
    LOG(WARNING) << "hit #" << i << "= " << fh;
    auto& d = digits[i];
    d.dualSampaId=i+1;
    d.dualSampaChannel=2;
    d.charge=3;
  }
}

of::AlgorithmSpec::ProcessCallback initDigitizer(of::InitContext& itx)
{
  auto detElemId = itx.options().get<int>("deid");

  Segmentation seg(detElemId, true);

  LOG(WARNING) << "detElemId=" << detElemId;

  return [seg, detElemId](of::ProcessingContext& ctx) {

    auto ref = ctx.inputs().get("mch-hits");
    auto hits = of::DataRefUtils ::as<FakeHit>(ref);
    LOG(WARNING) << "digitizer #input hits=" << hits.size();
    auto digits = ctx.outputs().make<FakeDigit>(of::Output{ "MCH", "DIGITS",
    static_cast<o2::header::DataHeader::SubSpecificationType>(detElemId) }, static_cast<size_t>(hits.size()));
    digitize(seg, hits, digits);
    sleep(1);
  };
}

of::AlgorithmSpec::ProcessCallback initHitGenerator(of::InitContext& itx)
{
  auto detElemId = itx.options().get<int>("deid");

  Segmentation seg(detElemId, true);

  auto nhits = itx.options().get<int>("nofhitperde");

  LOG(WARNING) << "detElemId=" << detElemId << " nhits=" << nhits;

  return [seg, nhits, detElemId](of::ProcessingContext& ctx) {
    auto outputHits = ctx.outputs().make<FakeHit>(
      of::Output{ "MCH", "HITS", static_cast<o2::header::DataHeader::SubSpecificationType>(detElemId) },
      static_cast<size_t>(nhits));
    generateFakeHits(outputHits, seg);
    sleep(1);
  };
}

of::ConfigParamSpec detElemIdOption(const char* help, int detElemId = 512)
{
  return of::ConfigParamSpec("deid", of::VariantType::Int, detElemId, { help });
}

of::ConfigParamSpec nofHitPerDEOption(const char* help, int nofHitPerDE = 10)
{
  return of::ConfigParamSpec("nofhitperde", of::VariantType::Int, nofHitPerDE, { help });
}

of::Inputs noInput{};
of::Outputs noOutput{};

of::DataProcessorSpec hitGeneratorSpec(uint detElemId)
{
  return {
    // clang-format off
      "mch-hit-generator",
      noInput,
      of::Outputs{{"MCH", "HITS", detElemId, of::Lifetime::Timeframe}},
      of::AlgorithmSpec{initHitGenerator},
      { detElemIdOption("which detection element to generate hits for"),
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
      of::Inputs{{"mch-hits", "MCH", "HITS", detElemId, of::Lifetime::Timeframe}},
      noOutput,
      of::AlgorithmSpec{[](of::ProcessingContext& ctx) {
        LOG(INFO) << "hit dump";
        auto hits = ctx.inputs().get("mch-hits");
        LOG(INFO) << "Header=" << hits.header;
        hits.spec->origin.print();
      }},
      { detElemIdOption("which detection element to dump hit for") }
    // clang-format on
  };
}

 of::DataProcessorSpec digitizerSpec(uint detElemId)
{
  return {
    // clang-format off
      "mch-digitizer",
      of::Inputs{{"mch-hits", "MCH", "HITS", detElemId, of::Lifetime::Timeframe}},
      of::Outputs{{"MCH","DIGITS",detElemId, of::Lifetime::Timeframe}},
      of::AlgorithmSpec{initDigitizer},
      { detElemIdOption("which detection element to digitize") }
    // clang-format on
  };
}

of::InjectorFunction digo2digi(of::OutputSpec spec) {

  return of::incrementalConverter(spec,0,1);
}

of::DataProcessorSpec digotizerSpec(uint detElemId)
{
  of::OutputSpec outspec{ "MCH", "DIGITS", detElemId, of::Lifetime::Timeframe };

  return of::specifyExternalFairMQDeviceProxy(
    // clang-format off
    "mch-digotizer",
    of::Outputs{outspec},
    "type=sub,method=connect,address=tcp://localhost:6060,rateLogging=1",
    digo2digi(outspec));
  // clang-format on
}

of::DataProcessorSpec preclusterizerSpec(uint detElemId)
{
  return {
    // clang-format off
      "mch-preclusterizer",
      of::Inputs{{"mch-digits", "MCH", "DIGITS", detElemId, of::Lifetime::Timeframe}},
      noOutput,
      of::AlgorithmSpec{[](of::ProcessingContext& ctx) {
        auto refs = ctx.inputs().get("mch-digits");
        auto digits = of::DataRefUtils::as<FakeDigit>(refs);
        LOG(WARNING) << "preclusterizer: digit dump " << digits.size() << " digits";
        for ( auto d: digits) {
          LOG(WARNING) << d;
        }
      }},
      { detElemIdOption("which detection element to preclusterize") }
    // clang-format on
  };
}

of::WorkflowSpec hitGeneratorWorkflow(uint detElemId)
{
  return {
    // clang-format off
      hitGeneratorSpec(detElemId),
      hitDumper(detElemId)
    // clang-format on
  };
}

of::WorkflowSpec digitizerWorkflow(uint detElemId)
{
  return {
      // clang-format off
      hitGeneratorSpec(detElemId),
      digotizerSpec(detElemId),
     // digitizerSpec(detElemId),
      preclusterizerSpec(detElemId)
      // clang-format on
  };
}

of::WorkflowSpec defineDataProcessing(const of::ConfigContext& configContext)
{
  //of::WorkflowSpec workflow{ hitGeneratorWorkflow(512) };
  return digitizerWorkflow(512);
}
