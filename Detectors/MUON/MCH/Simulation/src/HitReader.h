// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_SIMULATION_HITREADER_H
#define O2_MCH_SIMULATION_HITREADER_H

#include "Framework/Task.h"
#include <string>
#include "TBranch.h"
#include <vector>
#include "MCHSimulation/Hit.h"
#include <memory>

namespace o2
{
namespace mch
{

class TreeBranchWrapper;

class HitReader 
{
 public:
  HitReader(const char* fileName);

  ~HitReader();

  void init(o2::framework::InitContext& ic);

  void run(o2::framework::ProcessingContext& pc);

 private:
  std::string mFileName{""};
  std::unique_ptr<TreeBranchWrapper> mBranchWrapper{nullptr};
  std::vector<o2::mch::Hit>* mHits{nullptr};
  int mEntry{-1};
};

} // namespace mch
} // namespace o2
#endif