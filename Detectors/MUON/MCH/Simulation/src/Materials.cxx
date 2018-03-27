// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include <array>
#include "MCHSimulation/Materials.h"

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

void CreateSlatGas(int fieldType, float maxField)
{
  // Ar 80% + CO2 20%
  const int n = 3;
  std::array<float, n> a{ kAArgon, kACarbon, kAOxygen };
  std::array<float, n> z{ kZArgon, kZCarbon, kZOxygen };
  std::array<float, n> w{ .8, 1. / 15, 2. / 15 }; // Relative weight of each atom in the gas
  float d{ 0.001821 };

  int imat = autoIncrementedMaterialId();
  materialManager().Mixture(moduleName, imat, "Ar 80% + CO2 20%", a.data(), z.data(), d, n, w.data());

  materialManager().Medium(moduleName, Medium::SlatGas, "Ar 80% + CO2 20%", imat, 1, fieldType, maxField, kMaxfd,
                           kStemax, kDeemax, kEpsil, kStmin);
}

void CreateCarbon(int fieldType, float maxField)
{
  // for support panel and slat panel
  int imat = autoIncrementedMaterialId();
  materialManager().Material(moduleName, imat, "Carbon", kACarbon, kZCarbon, kDensCarbon, kRadCarbon, kAbsCarbon);
  materialManager().Medium(moduleName, Medium::Carbon, "Carbon", imat, 0, fieldType, maxField, kMaxfd, kStemax, kDeemax,
                           kEpsil, kStmin);
}

void CreateNomex(int fieldType, float maxField)
{
  // Nomex (honey comb) : C22 H10 N2 O5
  const int n = 4;
  std::array<float, n> a{ kACarbon, kAHydrogen, kANitrogen, kAOxygen };
  std::array<float, n> z{ kZCarbon, kZHydrogen, kZNitrogen, kZOxygen };
  std::array<float, n> w{ 22., 10., 2., 5. }; // Relative weight of each atom in the compound
  float d{ 0.024 };

  int imat = autoIncrementedMaterialId();
  // By giving a negative number of different atoms, it will compute itself the relative proportions of each atom so
  // that the total weight is equal to 1.
  materialManager().Mixture(moduleName, imat, "Nomex", a.data(), z.data(), d, -n, w.data());
  materialManager().Medium(moduleName, Medium::Nomex, "Nomex", imat, 1, fieldType, maxField, kMaxfd, kStemax, kDeemax,
                           kEpsil, kStmin);
}

void CreateNomexBulk(int fieldType, float maxField)
{
  // Nomex (bulk) : C22 H10 N2 O5
  const int n = 4;
  std::array<float, n> a{ kACarbon, kAHydrogen, kANitrogen, kAOxygen };
  std::array<float, n> z{ kZCarbon, kZHydrogen, kZNitrogen, kZOxygen };
  std::array<float, n> w{ 22., 10., 2., 5. }; // Relative weight of each atom in the compound
  float d{ 1.43 };

  int imat = autoIncrementedMaterialId();
  // By giving a negative number of different atoms, it will compute itself the relative proportions of each atom so
  // that the total weight is equal to 1.
  materialManager().Mixture(moduleName, imat, "NomexBulk", a.data(), z.data(), d, -n, w.data());
  materialManager().Medium(moduleName, Medium::NomexBulk, "NomexBulk", imat, 1, fieldType, maxField, kMaxfd, kStemax,
                           kDeemax, kEpsil, kStmin);
}

void CreateVacuum(int fieldType, float maxField)
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
    throw std::runtime_error("Could not retrieve medium " + std::to_string(imed) + " for " + impl::moduleName);
  }
  return med;
}

void CreateSlatGeometryMaterials()
{

  int fieldType;                                                // magnetic field type
  float maxField;                                               // maximum magnetic field value
  Base::Detector::initFieldTrackingParams(fieldType, maxField); // get the values

  impl::CreateVacuum(fieldType, maxField); // necessary ?
  impl::CreateSlatGas(fieldType, maxField);
  impl::CreateCarbon(fieldType, maxField);
  impl::CreateNomex(fieldType, maxField);
  impl::CreateNomexBulk(fieldType, maxField);
}

} // namespace mch
} // namespace o2
