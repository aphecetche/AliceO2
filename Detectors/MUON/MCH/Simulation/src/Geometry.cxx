// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "Geometry.h"

#include "Station1Geometry.h"
#include "Station2Geometry.h"
#include "Station345Geometry.h"
#include "Materials.h"
#include <iostream>
#include "TGeoVolume.h"

namespace o2
{
namespace mch
{

void createGeometry()
{
  createMaterials();
  createStation1Geometry();
  createStation2Geometry();
  createStation345Geometry();
}

std::vector<TGeoVolume*> getSensitiveVolumes()
{
  auto st1 = getStation1SensitiveVolumes();
  auto st2 = getStation2SensitiveVolumes();
  auto st345 = getStation345SensitiveVolumes();

  auto vol = st1;
  vol.insert(vol.end(), st2.begin(), st2.end());
  vol.insert(vol.end(), st345.begin(), st345.end());

  for (auto v: vol) {
      std::cout << v->GetName() << "\n";
  }
  return vol;
}

} // namespace mch
} // namespace o2