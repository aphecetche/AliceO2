// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file   SlatGeometry.h
/// \brief  Implementation of the slat stations geometry
/// \author Florian Damas <florian.damas@cern.ch>
/// \date   22 mars 2018

#ifndef O2_MCH_SIMULATION_SLATGEOMETRY_H
#define O2_MCH_SIMULATION_SLATGEOMETRY_H

#include <string>
#include <array>

using namespace std;

namespace o2
{
namespace mch
{

//  Constants
const Double_t kVertFrameLength = 2.; // space between a rounded PCB edge and the beam pipe ("dead zone")
const Double_t kVertSpacerLength = 2.5;
const Double_t kShortPCBlength = 35.;
const Double_t kGasDim[3] = { 40., 40., 0.5 };

const Double_t kSlatWidth = kGasDim[2]; // for the moment !!

// Inner radii
const Double_t kRadSt3 = 29.5;
const Double_t kRadSt45 = 37.5;

// Y position of the rounded slats
const Double_t kRoundedSlatYposSt3 = 37.8;
const Double_t kRoundedSlatYposSt45 = 38.2;

// Slat types
const array<string, 19> kSlatTypes{ "122000SR1", "112200SR2", "122200S", "222000N", "220000N",
                                    "122000NR1", "112200NR2", "122200N", "122330N", "112233NR3",
                                    "112230N",   "222330N",   "223300N", "333000N", "330000N",
                                    "112233N",   "222333N",   "223330N", "333300N" };

extern const string jsonSlatDescription;
void CreateSlatGeometry();
} // namespace mch
} // namespace o2
#endif
