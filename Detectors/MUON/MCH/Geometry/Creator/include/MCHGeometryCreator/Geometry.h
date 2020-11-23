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

#ifndef O2_MCH_GEOMETRY_CREATOR_GEOMETRY_H
#define O2_MCH_GEOMETRY_CREATOR_GEOMETRY_H

#include <vector>
#include <iostream>

class TGeoVolume;
class TGeoManager;

namespace o2::mch::geo
{

/// createGeometry creates MCH geometry and attach it to existing topVolume
void createGeometry(TGeoVolume& topVolume);

/// get a list of MCH sensitive volumes
std::vector<TGeoVolume*> getSensitiveVolumes();

/// Add alignable mch volumes
void addAlignableVolumes();

} // namespace o2::mch::geo

#endif
