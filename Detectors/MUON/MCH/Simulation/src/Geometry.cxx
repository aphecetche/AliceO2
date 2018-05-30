// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHSimulation/Geometry.h"

#include "Station1Geometry.h"
#include "Station2Geometry.h"
#include "Station345Geometry.h"
#include "Materials.h"
#include <iostream>
#include "TGeoVolume.h"
#include "TGeoManager.h"

namespace o2
{
namespace mch
{

TGeoVolume* createAirVacuumCave(const char* name)
{
  Float_t aAir[4] = { 12.0107, 14.0067, 15.9994, 39.948 };
  Float_t zAir[4] = { 6., 7., 8., 18. };
  Float_t wAir[4] = { 0.000124, 0.755267, 0.231781, 0.012827 };
  Float_t dAirVacuum = 1.20479E-10;
  gGeoManager->Mixture("air", aAir, zAir, dAirVacuum, 4, wAir, 1);
  TGeoMedium* mair = gGeoManager->Medium("air", 1, 1,
                                         false, /* isvol */
                                         0,     /* ifield */
                                         -1.0,  /* fieldm */
                                         -1.0,  /* tmaxfd */
                                         -1.0,  /* stemax */
                                         -1.0,  /* deemax */
                                         -1.0,  /* epsil */
                                         -1.0   /* stmin */
  );
  return gGeoManager->MakeBox(name, mair, 2000.0, 2000.0, 3000.0);
}

void createGeometry(TGeoVolume& topVolume)
{
  createMaterials();
  createStation1Geometry(topVolume);
  createStation2Geometry(topVolume);
  createStation345Geometry(topVolume);
}

std::vector<TGeoVolume*> getSensitiveVolumes()
{
  auto st1 = getStation1SensitiveVolumes();
  auto st2 = getStation2SensitiveVolumes();
  auto st345 = getStation345SensitiveVolumes();

  auto vol = st1;
  vol.insert(vol.end(), st2.begin(), st2.end());
  vol.insert(vol.end(), st345.begin(), st345.end());

  return vol;
}

void dump(const TGeoNode& n, int level, int maxdepth, std::string prefix)
{
  if (level >= maxdepth) {
    return;
  }

  if (level == 0) {
    std::cout << n.GetName() << "\n";
  }

  if (level < maxdepth) {
    for (int i = 0; i < n.GetNdaughters(); i++) {
      TGeoNode* d = n.GetDaughter(i);
      if (i == n.GetNdaughters() - 1) {
        std::cout << prefix + "└──" << d->GetName()
                  << "\n";
        dump(*d, level + 1, maxdepth, prefix + "   ");
      } else {
        std::cout << prefix + "├──" << d->GetName()
                  << "\n";
        dump(*d, level + 1, maxdepth, prefix + "│  ");
      }
    }
  }
}

void showGeometryAsTextTree(const char* fromPath, int maxdepth)
{
  if (!gGeoManager) {
    return;
  }

  TGeoNavigator* nav = gGeoManager->GetCurrentNavigator();

  if (strlen(fromPath)) {
    if (!nav->cd(fromPath)) {
      std::cerr << "Could not get path " << fromPath << "\n";
      return;
    }
  }

  TGeoNode* node = nav->GetCurrentNode();

  dump(*node, 0, maxdepth, "");
}

} // namespace mch
} // namespace o2