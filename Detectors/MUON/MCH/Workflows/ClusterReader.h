// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_WORKFLOW_CLUSTERREADER_H
#define O2_MCH_WORKFLOW_CLUSTERREADER_H

#include "Framework/Task.h"
#include <fstream>
#include <string>

namespace o2
{
namespace mch
{

class ClusterReader : public o2::framework::Task
{
 public:
  ClusterReader(const std::string& filename);

  void init(o2::framework::InitContext& context) override;
  void run(o2::framework::ProcessingContext& context) override;

 private:
  std::string mFileName{ "" };
  std::ifstream mInputFile;
};
}
}
#endif
