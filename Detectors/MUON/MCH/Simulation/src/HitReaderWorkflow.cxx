// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "Framework/WorkflowSpec.h"
#include "Framework/ConfigContext.h"

#include "HitReader.h"
#include <string>
#include <iostream>

namespace of = o2::framework;

void customize(std::vector<o2::framework::ConfigParamSpec>& workflowOptions)
{
  workflowOptions.push_back(
    of::ConfigParamSpec{ "hitfile", of::VariantType::String, "o2sim.root", { "ROOT file containing MCH hits" } });
}

#include "Framework/runDataProcessing.h"

of::WorkflowSpec defineDataProcessing(const of::ConfigContext& configContext)
{
  of::WorkflowSpec workflow;

  auto hitFileName = configContext.options().get<std::string>("hitfile");

  workflow.emplace_back(of::createSamplerSpec(
    "HitReader",
    of::adaptFromTask<o2::mch::HitReader>(hitFileName.c_str()),
    of::Outputs{ of::OutputSpec{ "MUON", "HITS" } }));

  workflow.emplace_back(of::DataProcessorSpec{
    "Digitizer",
    { of::InputSpec{ "mchhits", "MUON", "HITS" } },
    { of::OutputSpec{ "MUON", "DIGITS" } },
    of::AlgorithmSpec{
      [](of::ProcessingContext& pc) {
        std::cout << "would convert hit to digits here\n";
        auto hits = pc.inputs().get<int>("mchhits");
        static int i = 0;
        auto digits = pc.outputs().make<int>(of::Output{ "MUON", "DIGITS" }, i++);
      } } });

  workflow.emplace_back(of::createSinkSpec(
    "DigitDumper",
    { of::InputSpec{ "mchdigits", "MUON", "DIGITS" } },
    of::AlgorithmSpec{ [](of::ProcessingContext& pc) {
      std::cout << "would consume digits here\n";
    } }));

  return workflow;
}