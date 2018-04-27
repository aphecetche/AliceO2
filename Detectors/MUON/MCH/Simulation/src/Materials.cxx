// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file   Materials.h
/// \brief  Implementation of the MCH materials definitions
/// \author Florian Damas <florian.damas@cern.ch>
/// \date   22 mars 2018

#include "MCHSimulation/Materials.h"

using namespace std;

namespace o2
{
namespace mch
{
namespace impl
{
const char* moduleName{ "MCH" };

int autoIncrementedMaterialId()
{
  static int nmat{ 0 };
  return ++nmat;
}

void createSlatGas(int fieldType, float maxField)
{
  // Ar 80% + CO2 20%
  const int n = 3;
  float a[n] = { kAArgon, kACarbon, kAOxygen };
  float z[n] = { kZArgon, kZCarbon, kZOxygen };
  float w[n] = { 0.8, 1. / 15, 2. / 15 }; // Relative weight of each atom in the gas
  float d = 0.001821;                     // according to AliRoot

  int imat = autoIncrementedMaterialId();
  materialManager().Mixture(moduleName, imat, "Ar 80% + CO2 20%", a, z, d, n, w);
  materialManager().Medium(moduleName, Medium::SlatGas, "Ar 80% + CO2 20%", imat, 1, fieldType, maxField, kMaxfd,
                           kStemax, kDeemax, kEpsil, kStmin);
}

void createCarbon(int fieldType, float maxField)
{
  // for support panel and slat panel
  int imat = autoIncrementedMaterialId();
  materialManager().Material(moduleName, imat, "Carbon", kACarbon, kZCarbon, kDensCarbon, 0., 0.);
  materialManager().Medium(moduleName, Medium::Carbon, "Carbon", imat, 0, fieldType, maxField, kMaxfd, kStemax, kDeemax,
                           kEpsil, kStmin);
}

void createNomex(int fieldType, float maxField)
{
  // Nomex (honey comb) : C14 H10 N2 O2 (changed w.r.t AliRoot)
  const int n = 4;
  float a[n] = { kACarbon, kAHydrogen, kANitrogen, kAOxygen };
  float z[n] = { kZCarbon, kZHydrogen, kZNitrogen, kZOxygen };
  float w[n] = { 14., 10., 2., 2. };
  float d = 0.024; // according to AliRoot

  int imat = autoIncrementedMaterialId();
  materialManager().Mixture(moduleName, imat, "Nomex", a, z, d, -n, w);
  materialManager().Medium(moduleName, Medium::Nomex, "Nomex", imat, 0, fieldType, maxField, kMaxfd, kStemax, kDeemax,
                           kEpsil, kStmin);
}

void createNomexBulk(int fieldType, float maxField)
{
  // Nomex (bulk) : C14 H10 N2 O2 (changed w.r.t AliRoot)
  const int n = 4;
  float a[n] = { kACarbon, kAHydrogen, kANitrogen, kAOxygen };
  float z[n] = { kZCarbon, kZHydrogen, kZNitrogen, kZOxygen };
  float w[n] = { 14., 10., 2., 2. };
  float d = 1.43; // according to AliRoot

  int imat = autoIncrementedMaterialId();
  materialManager().Mixture(moduleName, imat, "NomexBulk", a, z, d, -n, w);
  materialManager().Medium(moduleName, Medium::NomexBulk, "NomexBulk", imat, 0, fieldType, maxField, kMaxfd, kStemax,
                           kDeemax, kEpsil, kStmin);
}

void createNoryl(int fieldType, float maxField)
{
  // Noryl 731 (ALICE-INT-2002-17) : C8 H8 O
  const int n = 3;
  float a[n] = { kACarbon, kAHydrogen, kAOxygen };
  float z[n] = { kZCarbon, kZHydrogen, kZOxygen };
  float w[n] = { 8., 8., 1. };
  float d = 1.06;

  int imat = autoIncrementedMaterialId();
  materialManager().Mixture(moduleName, imat, "Noryl", a, z, d, -n, w);
  materialManager().Medium(moduleName, Medium::Noryl, "Noryl", imat, 0, fieldType, maxField, kMaxfd, kStemax, kDeemax,
                           kEpsil, kStmin);
}

void createCopper(int fieldType, float maxField)
{
  int imat = autoIncrementedMaterialId();
  materialManager().Material(moduleName, imat, "Copper", kACopper, kZCopper, kDensCopper, 0., 0.);
  materialManager().Medium(moduleName, Medium::Copper, "Copper", imat, 0, fieldType, maxField, kMaxfd, kStemax, kDeemax,
                           kEpsil, kStmin);
}

void createG10(int fieldType, float maxField)
{
  // G10: SiO2(60%) + C8H14O4(40%) -> weights to be checked !!!
  const int n = 5;
  float a[n] = { kASilicon, kAOxygen, kACarbon, kAHydrogen, kAOxygen };
  float z[n] = { kZSilicon, kZOxygen, kZCarbon, kZHydrogen, kZOxygen };
  float w[n] = { 1 * 0.6, 2 * 0.6, 8 * 0.4, 14 * 0.4, 4 * 0.4 }; // Relative weight of each atom
  float d = 1.7;                                                 // according to AliRoot

  int imat = autoIncrementedMaterialId();
  materialManager().Mixture(moduleName, imat, "G10", a, z, d, -n, w);
  materialManager().Medium(moduleName, Medium::G10, "G10", imat, 0, fieldType, maxField, kMaxfd, kStemax, kDeemax,
                           kEpsil, kStmin);
}

void createRohacell(int fieldType, float maxField)
{
  // rohacell: C9 H13 N1 O2
  const int n = 4;
  float a[n] = { kACarbon, kAHydrogen, kANitrogen, kAOxygen };
  float z[n] = { kZCarbon, kZHydrogen, kZNitrogen, kZOxygen };
  float w[n] = { 9., 13., 1., 2. };
  float d = 0.03; // according to AliRoot

  int imat = autoIncrementedMaterialId();
  materialManager().Mixture(moduleName, imat, "Rohacell", a, z, d, -n, w);
  materialManager().Medium(moduleName, Medium::Rohacell, "Rohacell", imat, 0, fieldType, maxField, kMaxfd, kStemax,
                           kDeemax, kEpsil, kStmin);
}

void createGlue(int fieldType, float maxField)
{
  // araldite 2011 (ALICE-INT-2002-17) : C10 H25 N3
  const int n = 3;
  float a[n] = { kACarbon, kAHydrogen, kANitrogen };
  float z[n] = { kZCarbon, kZHydrogen, kZNitrogen };
  float w[n] = { 10., 25., 3. };
  float d = 1.066;

  int imat = autoIncrementedMaterialId();
  materialManager().Mixture(moduleName, imat, "Glue", a, z, d, -n, w);
  materialManager().Medium(moduleName, Medium::Glue, "Glue", imat, 0, fieldType, maxField, kMaxfd, kStemax, kDeemax,
                           kEpsil, kStmin);
}

void createVacuum(int fieldType, float maxField)
{
  int imat = autoIncrementedMaterialId();
  materialManager().Material(moduleName, imat, "Vacuum", 0, 0, 0, 0, 0, 0);
  materialManager().Medium(moduleName, Medium::Vacuum, "Vacuum", imat, 0, fieldType, maxField, -1, -1, -1, -1, -1);
}

} // namespace impl

o2::Base::MaterialManager& materialManager() { return o2::Base::MaterialManager::Instance(); }

TGeoMedium* assertMedium(int imed)
{
  auto med = materialManager().getTGeoMedium(impl::moduleName, imed);
  if (med == nullptr) {
    throw runtime_error("Could not retrieve medium " + to_string(imed) + " for " + impl::moduleName);
  }
  return med;
}

void createSlatGeometryMaterials()
{

  int fieldType;                                                // magnetic field type
  float maxField;                                               // maximum magnetic field value
  Base::Detector::initFieldTrackingParams(fieldType, maxField); // get the values

  // impl::createVacuum(fieldType, maxField); // necessary ? The stations are in another volume (cave)
  impl::createSlatGas(fieldType, maxField); // sensitive medium for tracking
  impl::createCarbon(fieldType, maxField);
  impl::createNomex(fieldType, maxField);
  impl::createNomexBulk(fieldType, maxField);
  impl::createNoryl(fieldType, maxField);    // spacers
  impl::createCopper(fieldType, maxField);   // PCB and cable medium
  impl::createG10(fieldType, maxField);      // insulator
  impl::createRohacell(fieldType, maxField); // for horizontal border
  impl::createGlue(fieldType, maxField);
}

} // namespace mch
} // namespace o2
