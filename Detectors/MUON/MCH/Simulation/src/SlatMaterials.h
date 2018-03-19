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

#ifndef O2_MCH_SIMULATION_SLATMATERIALS_H
#define O2_MCH_SIMULATION_SLATMATERIALS_H

#include "DetectorsBase/MaterialManager.h"

namespace o2
{
namespace mch
{
enum Medium {
  Vacuum,
  ArCo2,
};

// Return a pointer to the mch medium number imed.
// Throws an exception if imed is not within Medium enum
// and / or medium has not been defined yet.
TGeoMedium* assertMedium(int imed);

o2::Base::MaterialManager& materialManager();

void createSlatMaterials();

} // namespace mch
} // namespace o2

#endif
