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
#include "Framework/ConfigParamSpec.h"
#include "ClusterReader.h"
#include "ClusterWriter.h"

namespace of = o2::framework;

// add workflow options, note that customization needs to be declared before
// including Framework/runDataProcessing
void customize(std::vector<of::ConfigParamSpec>& workflowOptions)
{
  workflowOptions.push_back(
    of::ConfigParamSpec{ "preclusterfile", of::VariantType::String, "clusters-100.dat", { "binary (flatbuffers style) file containing MCH preclusters" } });

  workflowOptions.push_back(
    of::ConfigParamSpec{ "clusterfile", of::VariantType::String, "clusters-100.dat", { "binary (flatbuffers style) file containing MCH clusters" } });
}

#include "Framework/runDataProcessing.h" // the main driver

of::DataProcessorSpec createPreClusterSampler(const std::string& preClusterFileName, unsigned int detElemId)
{
  return of::DataProcessorSpec{ "mch-precluster-reader",
                                {},
                                { of::OutputSpec({ "PRECLUSTERS" }, o2::header::gDataOriginMCH, "preclusters", detElemId, of::Lifetime::Timeframe) },
                                of::adaptFromTask<o2::mch::ClusterReader>(preClusterFileName) };
}

// of::DataProcessorSpec createClusterizer()
// {
//   return of::DataProcessorSpec{ "mch-clusterizer",
//                                 { of::InputSpec() },
//                                 { of::OutputSpec({ "CLUSTERS" }, o2::header::gDataOriginMCH, "clusters", detElemId, of::Lifetime::Timeframe) },
//                                 of::adaptFromTask<o2::mch::Clusterizer>() };
// }
//
of::DataProcessorSpec createClusterWriter(const std::string& clusterFileName, unsigned int detElemId)
{
  return of::DataProcessorSpec{ "mch-cluster-writer",
                                { of::InputSpec{ "PRECLUSTERS", o2::header::gDataOriginMCH, "preclusters", detElemId, of::Lifetime::Timeframe } },
                                {},
                                of::adaptFromTask<o2::mch::ClusterWriter>(clusterFileName) };
}

of::WorkflowSpec defineDataProcessing(const of::ConfigContext& cc)
{
  of::WorkflowSpec workflow;

  auto preClusterFileName = cc.options().get<std::string>("preclusterfile");
  auto clusterFileName = cc.options().get<std::string>("clusterfile");

  unsigned int detElemId{ 100 };
  workflow.push_back(createPreClusterSampler(preClusterFileName,detElemId));
//   workflow.push_back(createClusterizer());
  workflow.push_back(createClusterWriter(clusterFileName,detElemId));

  return workflow;
}

