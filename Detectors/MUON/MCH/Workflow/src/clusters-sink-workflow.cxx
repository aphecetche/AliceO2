// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file clusters-sink-workflow.cxx
/// \brief This is an executable that dumps to a file on disk the clusters received via DPL.
///
/// \author Philippe Pillot, Subatech

#include "Framework/WorkflowSpec.h"
#include "Framework/ConfigParamSpec.h"

// add workflow options, note that customization needs to be declared before
// including Framework/runDataProcessing
void customize(std::vector<o2::framework::ConfigParamSpec>& workflowOptions)
{
  std::vector<o2::framework::ConfigParamSpec> options{
    {"global", o2::framework::VariantType::Bool, false, {"read clusters with positions expressed in global reference frame"}},
  };
  std::swap(workflowOptions, options);
}

#include "Framework/runDataProcessing.h"

#include "ClusterSinkSpec.h"

using namespace o2::framework;

WorkflowSpec defineDataProcessing(const ConfigContext& cc)
{
  return WorkflowSpec{o2::mch::getClusterSinkSpec(cc.options().get<bool>("global"))};
}
