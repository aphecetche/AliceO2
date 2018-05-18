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
#include "SlatGeometry.h"
#include "Stepper.h"
#include "TGeoManager.h"
#include <sstream>
#include <iostream>
#include <array>
#include "TGeoManager.h"

ClassImp(o2::mch::Detector);

namespace
{
std::string chamberName(int chamberNumber, bool inside)
{
  std::ostringstream name;
  name << "SC" << std::setw(2) << std::setfill('0') << chamberNumber << (inside ? "I" : "O");
  return name.str();
}
void dumpGeometry()
{
  // try to find back volume of slats
  for (auto ch = 5; ch <= 10; ++ch) {
    for (auto inside : std::array<bool, 2>{ true, false }) {
      auto chname = chamberName(ch, inside);
      auto vol = gGeoManager->GetVolume(chname.c_str());
      if (!vol) {
        std::cout << "could not get volume " << chname << std::endl;
        continue;
      }
      TIter next(vol->GetNodes());
      while (TGeoNode* node = static_cast<TGeoNode*>(next())) {
        std::cout << node->GetName() << " index=" << node->GetIndex() << " number=" << node->GetNumber() << "\n";
      }
    }
  }
}
} // namespace
namespace o2
{
namespace mch
{

Detector::Detector(bool active)
  : o2::Base::DetImpl<Detector>("MCH", active), mStepper{ std::make_unique<o2::mch::Stepper>() }
{
}

Detector::~Detector() = default;

void Detector::defineSensitiveVolumes()
{
  for (auto* vol : getSlatSensitiveVolumes()) {
      AddSensitiveVolume(vol);
  }
}

void Detector::Initialize()
{
  defineSensitiveVolumes();
  o2::Base::Detector::Initialize();
}

void Detector::ConstructGeometry()
{
  createSlatGeometry();
  dumpGeometry();
}

Bool_t Detector::ProcessHits(FairVolume* v)
{
  mStepper->process(*fMC);
  return kTRUE;
}

std::vector<o2::mch::Hit>* Detector::getHits(int) { return nullptr; /*return mStepper->getHits();*/ }

void Detector::Register()
{
  // TODO : get another way to do I/O (i.e. separate concerns)

  mStepper->registerHits(addNameTo("Hit").c_str());
}

void Detector::EndOfEvent() { mStepper->resetHits(); }

} // namespace mch
} // namespace o2
