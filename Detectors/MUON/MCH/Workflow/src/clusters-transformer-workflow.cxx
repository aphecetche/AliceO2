// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>

#include <stdexcept>

#include "Framework/CallbackService.h"
#include "Framework/ConfigParamRegistry.h"
#include "Framework/ControlService.h"
#include "Framework/DataProcessorSpec.h"
#include "Framework/Lifetime.h"
#include "Framework/Output.h"
#include "Framework/Task.h"
#include "Framework/Logger.h"

#include "MCHBase/ClusterBlock.h"
#include "Framework/runDataProcessing.h"
#include "MCHGeometryTransformer/Transformations.h"
#include "MathUtils/Cartesian.h"

namespace o2
{
namespace mch
{

using namespace std;
using namespace o2::framework;

class ClusterTransformerTask
{
 public:
  //_________________________________________________________________________________________________
  void init(framework::InitContext& ic)
  {
    auto geoFile = ic.options().get<std::string>("geometry");
    std::ifstream in(geoFile);
    transformation = o2::mch::geo::transformationFromJSON(in);
  }

  //_________________________________________________________________________________________________
  void run(framework::ProcessingContext& pc)
  {
    // read the clusters (assumed to be in local reference frame) and
    // tranform them into master reference frame.

    // get the input clusters
    auto localClusters = pc.inputs().get<gsl::span<ClusterStruct>>("clusters");

    // create the output message
    auto clusters = pc.outputs().make<ClusterStruct>(Output{"MCH", "GLOBALCLUSTERS", 0, Lifetime::Timeframe}, localClusters.size());

    int i{0};
    // local 2 global for all cluster
    for (auto& c : localClusters) {
      auto deId = c.getDEId();
      std::cout << fmt::format("DE {:4d}", deId);
      std::cout << c << "\n";
      o2::math_utils::Point3D<float> local{c.x, c.y, c.z};
      auto t = transformation(deId);
      auto global = t(local);
      auto& gcluster = clusters[i];
      gcluster = c;
      gcluster.x = global.x();
      gcluster.y = global.y();
      gcluster.z = global.z();
      std::cout << gcluster << "\n";
      i++;
    }
  }

 public:
  o2::mch::geo::TransformationCreator transformation;
};

} // end namespace mch
} // end namespace o2

using namespace o2::framework;

WorkflowSpec defineDataProcessing(const ConfigContext& cc)
{
  return WorkflowSpec{
    DataProcessorSpec{
      "ClusterTransformer",
      Inputs{InputSpec{"clusters", "MCH", "CLUSTERS", 0, Lifetime::Timeframe}},
      Outputs{OutputSpec{"MCH", "GLOBALCLUSTERS", 0, Lifetime::Timeframe}},
      AlgorithmSpec{adaptFromTask<o2::mch::ClusterTransformerTask>()},
      Options{
        {"geometry", VariantType::String, "geometry-o2.json", {"input geometry (json format)"}}}}};
}
