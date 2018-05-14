// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHSimulation/Detector.h"

#include "MCHSimulation/SlatGeometry.h"

#include "TGeoManager.h"
#include "TVirtualMC.h"
#include <stdexcept>

#include "TStopwatch.h"

ClassImp(o2::mch::Detector);

namespace o2
{
namespace mch
{

Detector::Detector(bool active) : o2::Base::DetImpl<Detector>("MCH", active) {}

void Detector::defineSensitiveVolumes()
{

  for (const auto& pcb : kPcbTypes) {

    const auto& volName = pcb.first;
    auto vol = gGeoManager->GetVolume(Form("%s gas", volName.data()));

    if (!vol) {
      LOG(WARNING) << std::string("could not get expected volume ") + std::string(volName);
    } else {
      AddSensitiveVolume(vol);
    }
  }
}

void Detector::Initialize()
{
  defineSensitiveVolumes();
  o2::Base::Detector::Initialize();
}

void Detector::ConstructGeometry()
{

  TStopwatch timer;
  createSlatGeometry();
  timer.Print("m");
}

Bool_t Detector::ProcessHits(FairVolume* v)
{
  int copy;
  int id = fMC->CurrentVolID(copy);

  std::cout << fMC->CurrentVolPath() << " id=" << id << " copy=" << copy << "\n";

  if (fMC->IsTrackEntering()) {
    std::cout << "Track entering\n";
  }
  if (fMC->IsTrackExiting()) {
    std::cout << "Track exiting\n";
  }
  return kTRUE;
}

} // namespace mch
} // namespace o2
