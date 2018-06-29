// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "HitReaderSpec.h"
#include "Framework/AlgorithmSpec.h"
#include "Framework/InitContext.h"
#include "Framework/ControlService.h"
#include "MCHSimulation/Hit.h"
#include "TFile.h"
#include "TTree.h"
#include <iostream>

namespace of = o2::framework;

namespace o2
{
namespace mch
{

namespace impl
{

TFile& getFile(std::string fileName)
{
  auto f = TFile::Open(fileName.c_str(), "READ");
  if (f->IsZombie()) {
    throw std::runtime_error("could not open file " + fileName);
  }
  return *f;
}

TTree& getTree(TFile& f, const char* treeName)
{
  TTree* tree = static_cast<TTree*>(f.Get(treeName));
  if (!tree) {
    throw std::runtime_error("could not get tree " + std::string(treeName));
  }
  return *tree;
}

TBranch& getBranch(TTree& t, const char* branchName)
{
  TBranch* b = t.GetBranch(branchName);
  if (!b) {
    throw std::runtime_error("could not get branch " + std::string(branchName));
  }
  return *b;
}

class TreeBranchWrapper
{
 private:
  TFile& mFile;
  TTree& mTree;
  TBranch& mBranch;

 public:
  TreeBranchWrapper(const char* fileName, const char* treeName, const char* branchName)
    : mFile(impl::getFile(fileName)),
      mTree(impl::getTree(mFile, treeName)),
      mBranch(impl::getBranch(mTree, branchName))
  {
    std::cout << "TreeBranchWrapper ctor " << this << "\n";
  }

  ~TreeBranchWrapper()
  {
    std::cout << "TreeBranchWrapper dtor " << this << "\n";
  }

  TBranch& getBranch() const { return mBranch; }
};

} // namespace impl

of::AlgorithmSpec::ProcessCallback hitReaderInitCallback(std::string hitFileName)
{
  auto tbw = std::make_shared<impl::TreeBranchWrapper>(hitFileName.c_str(), "o2sim", "MCHHit");
  return [tbw](of::ProcessingContext& pc) {
    static int i = 0;
    std::vector<o2::mch::Hit> *hits{nullptr};
    tbw->getBranch().SetAddress(&hits);
    tbw->getBranch().GetEntry(i);
    std::cout << "would work hard here to read " << hits->size() << " hits...\n";
    pc.outputs().make<int>(of::Output{ "MUON", "HITS" }, i++);
    sleep(1);
    if (i >= tbw->getBranch().GetEntries()) {
      LOG(INFO) << tbw->getBranch().GetEntries() << " branch entry reached. Will stop...\n";
      pc.services().get<of::ControlService>().readyToQuit(true);
    }
  };
}

} // namespace mch
} // namespace o2