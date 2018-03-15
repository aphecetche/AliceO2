// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

///
/// @author  Laurent Aphecetche

#include "MCHSimulation/Detector.h"

#include "SlatGeometry.h"
#include "TGeoManager.h"
#include "TVirtualMC.h"
#include <stdexcept>

ClassImp(o2::mch::Detector);

namespace o2
{
namespace mch
{

Detector::Detector(bool active) : o2::Base::DetImpl<Detector>("MCH", active) {}

void Detector::defineSensitiveVolumes()
{
  std::vector<std::string> sensitiveVolumeNames;

  sensitiveVolumeNames.emplace_back("centralPCB");
  sensitiveVolumeNames.emplace_back("uproundedPCB");
  sensitiveVolumeNames.emplace_back("downroundedPCB");

  for (auto i = 0; i < 6; ++i) {
    sensitiveVolumeNames.emplace_back(std::string("normalPCB") + std::to_string(i));
  }
  for (const auto& volName : sensitiveVolumeNames) {

    auto vol = gGeoManager->GetVolume(volName.c_str());

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

void Detector::ConstructGeometry() { createSlatGeometry(); }

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