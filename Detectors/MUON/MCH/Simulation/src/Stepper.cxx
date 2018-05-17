// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHSimulation/Stepper.h"

#include "SimulationDataFormat/Stack.h"
#include "SimulationDataFormat/TrackReference.h"
#include "TGeoManager.h"
#include "TVirtualMC.h"
#include "TVirtualMCStack.h"
#include "TMCProcess.h"
#include <iomanip>
#include "TArrayI.h"

#include "FairRootManager.h"

namespace o2
{
namespace mch
{

Stepper::Stepper() : mHits{ new std::vector<o2::mch::Hit>(20) } {}
Stepper::~Stepper()
{
  delete mHits;
  showProcesses();
}

void Stepper::showProcesses() {
  std::cout << "--- MCH Stepper : processes seen\n";
  for (int i = 0; i < kMaxMCProcess; i++) {
    if (mProcessCodes[i] > 0) {
      std::cout << "---" << std::setw(40) << TMCProcessName[i] << " " << std::setprecision(2) << 100.0*mProcessCodes[i]/mNofSteps << "% \n";
    }
  }
}

void Stepper::countProcesses(const TVirtualMC& vmc)
{
  TArrayI proCodes;
  vmc.StepProcesses(proCodes);

  for (int i = 0; i < proCodes.GetSize(); ++i) {
    mProcessCodes[proCodes.At(i)]++;
  }
}

void Stepper::process(const TVirtualMC& vmc)
{
  mNofSteps++;

  o2::SimTrackStatus t{ vmc };

  int detElemId;
  vmc.CurrentVolOffID(2, detElemId); // go up 2 levels in the hierarchy to get the DE

  auto stack = static_cast<o2::Data::Stack*>(vmc.GetStack());

  if (t.isEntering() || t.isExiting()) {
    // generate a track referenced
    o2::TrackReference tr{ vmc, detElemId };
    stack->addTrackReference(tr);
  }

  if (t.isEntering()) {
    resetStep();
    // float x,y,z;
    // vmc.TrackPosition(x,y,z);
  }

  mTrackEloss += vmc.Edep();
  mTrackLength += vmc.TrackStep();

  countProcesses(vmc);

  const float GeV2KeV = 1E6;

  if (t.isExiting() || t.isStopped()) {
      mHits->emplace_back(stack->GetCurrentTrackNumber(), detElemId, mTrackEloss * GeV2KeV, mTrackLength);
      resetStep();
  }
}

void Stepper::registerHits(const char* branchName)
{
  if (FairRootManager::Instance()) {
    FairRootManager::Instance()->RegisterAny(branchName, mHits, kTRUE);
  }
}

void Stepper::resetStep() {
    mTrackEloss = 0.0;
    mTrackLength = 0.0;
}

void Stepper::resetHits() { mHits->clear(); }

} // namespace mch
} // namespace o2
