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

void CreateSlatGas()
{
  // Ar 80% + CO2 20%
  const int n = 3;
  float a[n] = { kAArgon, kACarbon, kAOxygen };
  float z[n] = { kZArgon, kZCarbon, kZOxygen };
  float w[n] = { 0.8, 1. / 15, 2. / 15 }; // Relative weight of each atom in the gas
  float d = 0.001821;                     // according to AliRoot

  int imat = autoIncrementedMaterialId();
  materialManager().Mixture(moduleName, imat, "Ar 80% + CO2 20%", a, z, d, n, w);
  materialManager().Medium(moduleName, Medium::SlatGas, "Ar 80% + CO2 20%", imat, 1, kFieldType, kMaxField, kMaxfd,
                           kStemax, kDeemax, kEpsil, kStmin);
}

void CreateCarbon()
{
  // for panels
  int imat = autoIncrementedMaterialId();
  materialManager().Material(moduleName, imat, "Carbon", kACarbon, kZCarbon, kDensCarbon, 0., 0.);
  materialManager().Medium(moduleName, Medium::Carbon, "Carbon", imat, 0, kFieldType, kMaxField, kMaxfd, kStemax,
                           kDeemax, kEpsil, kStmin);
}

void CreateNomex()
{
  // Nomex (honey comb) : C14 H10 N2 O2 (changed w.r.t AliRoot)
  const int n = 4;
  float a[n] = { kACarbon, kAHydrogen, kANitrogen, kAOxygen };
  float z[n] = { kZCarbon, kZHydrogen, kZNitrogen, kZOxygen };
  float w[n] = { 14., 10., 2., 2. };
  float d = 0.024; // according to AliRoot

  int imat = autoIncrementedMaterialId();
  materialManager().Mixture(moduleName, imat, "Nomex", a, z, d, -n, w);
  materialManager().Medium(moduleName, Medium::Nomex, "Nomex", imat, 0, kFieldType, kMaxField, kMaxfd, kStemax, kDeemax,
                           kEpsil, kStmin);
}

void CreateNomexBulk()
{
  // Nomex (bulk) : C14 H10 N2 O2 (changed w.r.t AliRoot)
  const int n = 4;
  float a[n] = { kACarbon, kAHydrogen, kANitrogen, kAOxygen };
  float z[n] = { kZCarbon, kZHydrogen, kZNitrogen, kZOxygen };
  float w[n] = { 14., 10., 2., 2. };
  float d = 1.43; // according to AliRoot

  int imat = autoIncrementedMaterialId();
  materialManager().Mixture(moduleName, imat, "NomexBulk", a, z, d, -n, w);
  materialManager().Medium(moduleName, Medium::NomexBulk, "NomexBulk", imat, 0, kFieldType, kMaxField, kMaxfd, kStemax,
                           kDeemax, kEpsil, kStmin);
}

void CreateNoryl()
{
  // Noryl 731 (ALICE-INT-2002-17) : C8 H8 O
  const int n = 3;
  float a[n] = { kACarbon, kAHydrogen, kAOxygen };
  float z[n] = { kZCarbon, kZHydrogen, kZOxygen };
  float w[n] = { 8., 8., 1. };
  float d = 1.06;

  int imat = autoIncrementedMaterialId();
  materialManager().Mixture(moduleName, imat, "Noryl", a, z, d, -n, w);
  materialManager().Medium(moduleName, Medium::Noryl, "Noryl", imat, 0, kFieldType, kMaxField, kMaxfd, kStemax, kDeemax,
                           kEpsil, kStmin);
}

void CreateCopper()
{
  // for pcb's
  int imat = autoIncrementedMaterialId();
  materialManager().Material(moduleName, imat, "Copper", kACopper, kZCopper, kDensCopper, 0., 0.);
  materialManager().Medium(moduleName, Medium::Copper, "Copper", imat, 0, kFieldType, kMaxField, kMaxfd, kStemax,
                           kDeemax, kEpsil, kStmin);
}

void CreateG10()
{
  // G10: SiO2(60%) + C8H14O4(40%) -> weights to be checked !!!
  const int n = 5;
  float a[n] = { kASilicon, kAOxygen, kACarbon, kAHydrogen, kAOxygen };
  float z[n] = { kZSilicon, kZOxygen, kZCarbon, kZHydrogen, kZOxygen };
  float w[n] = { 1 * 0.6, 2 * 0.6, 8 * 0.4, 14 * 0.4, 4 * 0.4 }; // Relative weight of each atom
  float d = 1.7;                                                 // according to AliRoot

  int imat = autoIncrementedMaterialId();
  materialManager().Mixture(moduleName, imat, "G10", a, z, d, -n, w);
  materialManager().Medium(moduleName, Medium::G10, "G10", imat, 0, kFieldType, kMaxField, kMaxfd, kStemax, kDeemax,
                           kEpsil, kStmin);
}

void CreateRohacell()
{
  // rohacell: C9 H13 N1 O2
  const int n = 4;
  float a[n] = { kACarbon, kAHydrogen, kANitrogen, kAOxygen };
  float z[n] = { kZCarbon, kZHydrogen, kZNitrogen, kZOxygen };
  float w[n] = { 9., 13., 1., 2. };
  float d = 0.03; // according to AliRoot

  int imat = autoIncrementedMaterialId();
  materialManager().Mixture(moduleName, imat, "Rohacell", a, z, d, -n, w);
  materialManager().Medium(moduleName, Medium::Rohacell, "Rohacell", imat, 0, kFieldType, kMaxField, kMaxfd, kStemax,
                           kDeemax, kEpsil, kStmin);
}

void CreateGlue()
{
  // araldite 2011 (ALICE-INT-2002-17) : C10 H25 N3
  const int n = 3;
  float a[n] = { kACarbon, kAHydrogen, kANitrogen };
  float z[n] = { kZCarbon, kZHydrogen, kZNitrogen };
  float w[n] = { 10., 25., 3. };
  float d = 1.066;

  int imat = autoIncrementedMaterialId();
  materialManager().Mixture(moduleName, imat, "Glue", a, z, d, -n, w);
  materialManager().Medium(moduleName, Medium::Glue, "Glue", imat, 0, kFieldType, kMaxField, kMaxfd, kStemax, kDeemax,
                           kEpsil, kStmin);
}

void CreateVacuum()
{
  int imat = autoIncrementedMaterialId();
  materialManager().Material(moduleName, imat, "Vacuum", 0, 0, 0, 0, 0, 0);
  materialManager().Medium(moduleName, Medium::Vacuum, "Vacuum", imat, 0, kFieldType, kMaxField, -1, -1, -1, -1, -1);
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

void CreateSlatGeometryMaterials()
{

  /// Magnetic field values
  int fieldType;  // magnetic field type
  float maxField; // maximum magnetic field value

  Base::Detector::initFieldTrackingParams(fieldType, maxField); // get the values

  // define these values as constants for the medium definitions
  const int kFieldType = fieldType;
  const float kMaxField = maxField;

  // impl::CreateVacuum(); // necessary ? The stations are in another volume (cave)
  impl::CreateSlatGas(); // sensitive medium for tracking
  impl::CreateCarbon();
  impl::CreateNomex();
  impl::CreateNomexBulk();
  impl::CreateNoryl();    // spacers
  impl::CreateCopper();   // PCB and cable medium
  impl::CreateG10();      // insulator
  impl::CreateRohacell(); // for horizontal border
  impl::CreateGlue();
}

} // namespace mch
} // namespace o2
