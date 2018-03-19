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

#include <array>
#include "SlatMaterials.h"

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

void createArCo2(const char* name)
{
  // create the tracking gas medium to be placed in the slats (to be called at
  // least once and before a slat creation)

  // Gas medium definition (Ar 80% + CO2 20%)
  std::array<float, 3> aGas{ 39.95, 12.01, 16. };
  std::array<float, 3> zGas{ 18., 6., 8. };
  std::array<float, 3> wGas{ .8, .0667, .13333 };
  float dGas{ 0.001821 };

  int imat = autoIncrementedMaterialId();
  materialManager().Mixture(moduleName, imat, name, aGas.data(), zGas.data(), dGas, aGas.size(), wGas.data());

  int isvol{ 1 };
  int ifield{ 2 };
  float fieldm{ 5.0 };  // max mag field - FIXME: get this from o2 field ?
  float epsil{ 0.001 }; // Tracking precision,

  // negative values below means "let the MC transport code compute the values"
  float tmaxfd{ -1. }; // Maximum angle due to field deflection
  float stemax{ -1. }; // Maximum displacement for multiple scat
  float deemax{ -1. }; // Maximum fractional energy loss, DLS
  float stmin{ -1 };

  materialManager().Medium(moduleName, Medium::ArCo2, name, imat, isvol, ifield, fieldm, tmaxfd, stemax, deemax, epsil,
                           stmin);
}

void CreateVacuum(const char* name)
{
  int imat = autoIncrementedMaterialId();
  materialManager().Material(moduleName, imat, name, 0, 0, 0, 0, 0, 0);
  materialManager().Medium(moduleName, Medium::Vacuum, name, imat, 0, 0, 5.0, -1, -1, -1, -1, -1);
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
void createSlatMaterials()
{
  impl::CreateVacuum("Vacuum");
  impl::createArCo2("Ar 80% + CO2 20%");
}

} // namespace mch
} // namespace o2
