// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// @file   Geometry.h
/// @brief  Interface for MCH geometry creation

#ifndef O2_MCH_SIMULATION_GEOMETRY_H
#define O2_MCH_SIMULATION_GEOMETRY_H

#include <vector>

class TGeoVolume;

namespace o2
{
namespace mch
{

/// createGeometry creates MCH geometry and attach it to topVolume
void createGeometry(TGeoVolume& topVolume);

/// get a list of MCH sensitive volumes
std::vector<TGeoVolume*> getSensitiveVolumes();

/// convenience method to create a top volume if not done otherwise
TGeoVolume* createAirVacuumCave(const char* name="cave");

/// tree like textual dump of the geometry nodes
void showGeometryAsTextTree(const char* fromPath="", int maxdepth=2);

} // namespace mch
} // namespace o2

#endif // O2_MCH_SIMULATION_GEOMETRY_H
