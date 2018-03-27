// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_SIMULATION_SLATMATERIALS_H
#define O2_MCH_SIMULATION_SLATMATERIALS_H

#include "DetectorsBase/MaterialManager.h"
#include "DetectorsBase/Detector.h" // for the magnetic field

namespace o2
{
namespace mch
{

/// Definition of constants for the elements
/// The atomic number and the atomic masse values are taken from the 2016 PDG booklet
/// The other constants (density, radiation length , ...) come from AliRoot

// Hydrogen
const float kZHydrogen = 1.;
const float kAHydrogen = 1.00794;

// Carbon
const float kZCarbon = 6.;
const float kACarbon = 12.0107;
const float kDensCarbon = 2.265;
const float kRadCarbon = 18.8;
const float kAbsCarbon = 49.9;

// Nitrogen
const float kZNitrogen = 7.;
const float kANitrogen = 14.0067;

// Oxygen
const float kZOxygen = 8.;
const float kAOxygen = 15.9994;

// Argon
const float kZArgon = 18.;
const float kAArgon = 39.948;

/// Tracking parameters (values taken from AliMUONCommonGeometryBuilder)
const float kEpsil{ 0.001 }; // Tracking precision [cm]

// negative values below means "let the MC transport code compute the values"
const float kMaxfd{ -20. }; // Maximum deflection angle due to magnetic field
const float kStemax{ -1. }; // Maximum displacement for multiple scattering [cm]
const float kDeemax{ -.3 }; // Maximum fractional energy loss, DLS
const float kStmin{ -.8 };  // Minimum step due to continuous processes [cm]

enum Medium { Vacuum, SlatGas, Carbon, Nomex, NomexBulk };

// Return a pointer to the mch medium number imed.
// Throws an exception if imed is not within Medium enum
// and / or medium has not been defined yet.
TGeoMedium* assertMedium(int imed);

o2::Base::MaterialManager& materialManager();

void CreateSlatGeometryMaterials();

} // namespace mch
} // namespace o2

#endif
