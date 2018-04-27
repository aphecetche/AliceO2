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
/// \brief  Implementation of the slat-stations geometry
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

///  Constants

/// Gas
const float kGasLength = 40.;
const float kGasHeight = 40.;
const float kGasWidth = 2 * 0.25;

/// PCB
const float kPCBLength = kGasLength;
const float kShortPCBLength = 35.;
const float kRoundedPCBLength = 42.5;
const float kR1PCBLength = 19.25;
const float kPCBHeight = 58.;
const float kPCBWidth = 0.002; // changed w.r.t AliRoot after investigation

/// Insulator (FR4)
const float kInsuWidth = 0.04; // changed w.r.t AliRoot after investigation

/// PCB volume (gas + 2*(pcb plate + insulator))
const float kTotalPCBWidth = kGasWidth + 2 * (kPCBWidth + kInsuWidth);

/// Slat panel = honeycomb nomex + 2 carbon fiber skins
const float kSlatPanelHeight = 42.5; // according to construction plan, its length depends on the number of PCBs

/// Glue
const float kGlueWidth = 0.004; // according to construction plan

/// Bulk Nomex
const float kNomexBulkWidth = 0.025; // according to AliRoot and construction plan

/// Carbon fiber
const float kCarbonWidth = 0.02; // according to AliRoot and construction plan

/// Honeycomb Nomex
const float kNomexWidth = 0.8; // according to AliRoot and construction plan

/// Spacers
const float kSpacerWidth = kGasWidth;
// Horizontal
const float kHoriSpacerHeight = 1.95; // according to AliRoot and construction plan
// Vertical
const float kVertSpacerLength = 2.5; // according to AliRoot and construction plan

const float kRoundedSpacerLength = 2.; // according to AliRoot and construction plan

// Support panels (to be checked !!!)
const float kCarbonSupportWidth = 0.03;
const float kGlueSupportWidth = 0.02; // added w.r.t AliRoot to match the construction plans
const float kNomexSupportWidth = 1.5;
const float kSupportWidth = 2 * (kCarbonSupportWidth + kGlueSupportWidth) + kNomexSupportWidth;
const float kSupportHeightSt3 = 361.;
const float kSupportHeightSt4 = 530.;
const float kSupportHeightSt5 = 570.;
const float kSupportLengthCh5 = 162.;
const float kSupportLengthCh6 = 167.;
const float kSupportLengthSt45 = 260.;

// Inner radii
const float kRadSt3 = 29.5;
const float kRadSt45 = 37.5;

// Y position of the rounded slats
const float kRoundedSlatYposSt3 = 37.8;
const float kRoundedSlatYposSt45 = 38.2;

// PCB types
const array<string, 10> kPcbTypes{ "B1N1", "B2N2-", "B2N2+", "B3-N3", "B3+N3", "R1", "R2", "R3", "S2-", "S2+" };

// Slat types
const map<string, vector<string>> kSlatTypes = { { "122000SR1", { "R1", "B1N1", "B2N2+", "S2-" } },
                                                 { "112200SR2", { "R2", "B1N1", "B2N2+", "S2-" } },
                                                 { "122200S", { "B1N1", "B2N2-", "B2N2-", "S2+" } },
                                                 { "222000N", { "B2N2-", "B2N2-", "B2N2-" } },
                                                 { "220000N", { "B2N2-", "B2N2-" } },
                                                 { "122000NR1", { "R1", "B1N1", "B2N2+", "B2N2+" } },
                                                 { "112200NR2", { "R2", "B1N1", "B2N2+", "B2N2+" } },
                                                 { "122200N", { "B1N1", "B2N2-", "B2N2-", "B2N2-" } },
                                                 { "122330N", { "B1N1", "B2N2+", "B2N2-", "B3-N3", "B3-N3" } },
                                                 { "112233NR3", { "R3", "B1N1", "B2N2-", "B2N2+", "B3+N3", "B3+N3" } },
                                                 { "112230N", { "B1N1", "B1N1", "B2N2-", "B2N2-", "B3-N3" } },
                                                 { "222330N", { "B2N2+", "B2N2+", "B2N2-", "B3-N3", "B3-N3" } },
                                                 { "223300N", { "B2N2+", "B2N2-", "B3-N3", "B3-N3" } },
                                                 { "333000N", { "B3-N3", "B3-N3", "B3-N3" } },
                                                 { "330000N", { "B3-N3", "B3-N3" } },
                                                 { "112233N", { "B1N1", "B1N1", "B2N2+", "B2N2-", "B3-N3", "B3-N3" } },
                                                 { "222333N",
                                                   { "B2N2+", "B2N2+", "B2N2+", "B3-N3", "B3-N3", "B3-N3" } },
                                                 { "223330N", { "B2N2+", "B2N2+", "B3-N3", "B3-N3", "B3-N3" } },
                                                 { "333300N", { "B3+N3", "B3-N3", "B3-N3", "B3-N3" } } };

extern const string jsonSlatDescription;
void createSlatGeometry();
} // namespace mch
} // namespace o2
#endif
