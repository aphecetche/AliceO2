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

ClassImp(o2::mch::Detector);

namespace o2
{
namespace mch
{

Detector::Detector(bool active) : o2::Base::DetImpl<Detector>("MCH", active) {}

void Detector::defineSensitiveVolumes()
{
  std::vector<std::string> sensitiveVolumeNames;

  sensitiveVolumeNames.emplace_back("roundedPCB");
  sensitiveVolumeNames.emplace_back("normalPCB");
  sensitiveVolumeNames.emplace_back("shortPCB");

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

void Detector::ConstructGeometry()
{
  CreateMaterials();
  CreateSlatGeometry();
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

void Detector::CreateMaterials()
{

  Int_t matID = 0, medID = 0;      // counters to ensure that we don't overwrite
  Bool_t isUnsens = 0, isSens = 1; // sensitive or unsensitive medium

  Int_t fieldType;
  Float_t maxField;
  o2::Base::Detector::initFieldTrackingParams(fieldType, maxField);

  // Values taken from AliMUONCommonGeometryBuilder, to be updated ??
  Float_t epsil = .001;  // Tracking precision [cm]
  Float_t stemax = -1.;  // Maximum displacement for multiple scattering [cm]
  Float_t tmaxfd = -20.; // Maximum deflection angle due to magnetic field
  Float_t deemax = -.3;  // Maximum fractional energy loss, DLS
  Float_t stmin = -.8;   // Minimum step due to continuous processes [cm] (negative value: choose it automatically)
  /*
  Float_t  maxDestepAlu = fMUON->GetMaxDestepAlu();
  Float_t  maxDestepGas = fMUON->GetMaxDestepGas();
  Float_t  maxStepAlu = fMUON->GetMaxStepAlu();
  Float_t  maxStepGas = fMUON->GetMaxStepGas();
  */
  // To be replaced by numerical values ?!
  Float_t maxDestepAlu = 0.;
  Float_t maxDestepGas = 0.;
  Float_t maxStepAlu = 0.;
  Float_t maxStepGas = 0.;

  /// Materials

  // Hydrogen
  Float_t zHydro = 1.;
  Float_t aHydro = 1.008;

  // Carbon
  Float_t zCarbon = 6.;
  Float_t aCarbon = 12.0107;
  Float_t dCarbon = 2.265;
  Float_t radCarbon = 18.8; // radiation length
  Float_t absCarbon = 49.9;
  o2::Base::Detector::Material(++matID, "Carbon", aCarbon, zCarbon, dCarbon, radCarbon, absCarbon);
  o2::Base::Detector::Medium(++medID, "Carbon", matID, isUnsens, fieldType, maxField, tmaxfd, maxStepAlu, maxDestepAlu,
                             epsil, stmin);

  // Nitrogen
  Float_t zNitro = 7.;
  Float_t aNitro = 14.007;

  // Oxygen
  Float_t zOxygen = 8.;
  Float_t aOxygen = 15.999;

  // Argon
  Float_t zArgon = 18.;
  Float_t aArgon = 39.948;

  /// Slat station media

  //     Ar-CO2 gas (80%+20%)
  const Int_t nGas = 3;
  Float_t aGas[nGas] = { aArgon, aCarbon, aOxygen };
  Float_t zGas[nGas] = { zArgon, zCarbon, zOxygen };
  Float_t wGas[nGas] = { .8, .0667, .13333 };
  Float_t dGas = .001821;
  o2::Base::Detector::Mixture(++matID, "SlatGas", aGas, zGas, dGas, nGas, wGas);
  o2::Base::Detector::Medium(++medID, "SlatGas", matID, isSens, fieldType, maxField, tmaxfd, maxStepGas, maxDestepGas,
                             epsil, stmin);

  // Nomex: C22 H10 N2 O5
  const Int_t nNomex = 4;
  Float_t aNomex[nNomex] = { aCarbon, aHydro, aNitro, aOxygen };
  Float_t zNomex[nNomex] = { zCarbon, zHydro, zNitro, zOxygen };
  Float_t wNomex[nNomex] = { 22., 10., 2., 5. };
  Float_t dNomex = 0.024; // honey comb
  o2::Base::Detector::Mixture(++matID, "Nomex", aNomex, zNomex, dNomex, nNomex, wNomex);
  o2::Base::Detector::Medium(++medID, "Nomex", matID, isUnsens, fieldType, maxField, tmaxfd, maxStepAlu, maxDestepAlu,
                             epsil, stmin);

  Float_t dNomexBulk = 1.43; // bulk material
  o2::Base::Detector::Mixture(++matID, "NomexBulk", aNomex, zNomex, dNomexBulk, nNomex, wNomex);
  o2::Base::Detector::Medium(++medID, "NomexBulk", matID, isUnsens, fieldType, maxField, tmaxfd, maxStepAlu,
                             maxDestepAlu, epsil, stmin);
}

} // namespace mch
} // namespace o2
