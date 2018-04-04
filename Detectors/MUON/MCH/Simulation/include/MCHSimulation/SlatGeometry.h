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

///  Constants

/// Gas
const Double_t kGasLength = 40.;
const Double_t kGasHeight = 40.;
const Double_t kGasWidth = 0.5;

/// PCB
const Double_t kPCBLength = kGasLength;
const Double_t kShortPCBLength = 35.;
const Double_t kPCBHeight = 58.;
const Double_t kPCBWidth = 0.003; // according to AliRoot

/// Insulation (to be checked)
const Double_t kInsuLength = kPCBLength; // according to AliRoot
const Double_t kInsuHeight = kPCBHeight; // according to AliRoot
const Double_t kInsuWidth = 0.022;       // according to AliRoot

/// Bulk Nomex
const Double_t kNomexBulkWidth = 0.025; // according to AliRoot

/// Slat panel = 2 layers of 0.02cm of carbon + 0.8cm of nomex (honey comb)
const Double_t kCarbonPanelWidth = 0.02;                                   // according to AliRoot
const Double_t kNomexPanelWidth = 0.8;                                     // according to AliRoot
const Double_t kSlatPanelLength = kGasLength;                              // according to AliRoot
const Double_t kSlatPanelHeight = kGasHeight;                              // according to AliRoot
const Double_t kSlatPanelWidth = kNomexPanelWidth + 2 * kCarbonPanelWidth; // according to AliRoot

/// Spacers
// Horizontal
const Double_t kHoriSpacerLength = kPCBLength; // according to AliRoot
const Double_t kHoriSpacerHeight = 1.95;       // according to AliRoot
const Double_t kHoriSpacerWidth = kGasWidth;   // according to AliRoot
// Vertical
const Double_t kVertSpacerLength = 2.5;
const Double_t kVertSpacerWidth = kGasWidth; // according to AliRoot
const Double_t kVertSpacerHeight = kGasLength + kHoriSpacerHeight;

/// Slats
const Double_t kSlatHeight = kPCBHeight;
const Double_t kSlatWidth = kGasWidth + 2 * (kPCBWidth + kInsuWidth + kSlatPanelWidth + kNomexBulkWidth);

const Double_t kVertFrameLength = 2.; // space between a rounded PCB edge and the beam pipe ("dead zone")

// Support panels (to be checked !!!)
const Double_t kCarbonSupportWidth = 0.03; // changed w.r.t AliRoot to match the construction plans
const Double_t kGlueSupportWidth = 0.02;   // changed w.r.t AliRoot to match the construction plans
const Double_t kNomexSupportWidth = 1.7;   // changed w.r.t AliRoot to match the construction plans
const Double_t kSupportWidth = 2 * (kCarbonSupportWidth + kGlueSupportWidth) + kNomexSupportWidth;
const Double_t kSupportHeightSt3 = 361.;
const Double_t kSupportHeightSt4 = 530.;
const Double_t kSupportHeightSt5 = 570.;
const Double_t kSupportLengthCh5 = 162.;
const Double_t kSupportLengthCh6 = 167.;
const Double_t kSupportLengthSt45 = 260.;

// Inner radii
const Double_t kRadSt3 = 29.5;
const Double_t kRadSt45 = 37.5;

// Y position of the rounded slats
const Double_t kRoundedSlatYposSt3 = 37.8;
const Double_t kRoundedSlatYposSt45 = 38.2;

// PCB types
const array<string, 10> kPcbTypes{ "B1N1", "B2N2-", "B2N2+", "B3-N3", "B3+N3", "R1", "R2", "R3", "S2-", "S2+" };

// Slat types
const map<string, vector<string>> kSlatTypes = { { "122000SR1", { "R1", "B2N2+", "B1N1", "S2-" } },
                                                 { "112200SR2", { "R2", "B2N2+", "B1N1", "S2-" } },
                                                 { "122200S", { "B1N1", "B2N2-", "B2N2-", "S2+" } },
                                                 { "222000N", { "B2N2-", "B2N2-", "B2N2-" } },
                                                 { "220000N", { "B2N2-", "B2N2-" } },
                                                 { "122000NR1", { "R1", "B2N2+", "B2N2+", "B1N1" } },
                                                 { "112200NR2", { "R2", "B2N2+", "B2N2+", "B1N1" } },
                                                 { "122200N", { "B1N1", "B2N2-", "B2N2-", "B2N2-" } },
                                                 { "122330N", { "B1N1", "B2N2+", "B2N2-", "B3-N3", "B3-N3" } },
                                                 { "112233NR3", { "R3", "B3+N3", "B3+N3", "B2N2+", "B2N2-", "B1N1" } },
                                                 { "112230N", { "B1N1", "B1N1", "B2N2-", "B2N2-", "B3-N3" } },
                                                 { "222330N", { "B2N2+", "B2N2+", "B2N2-", "B3-N3", "B3-N3" } },
                                                 { "223300N", { "B2N2+", "B2N2-", "B3-N3", "B3-N3" } },
                                                 { "333000N", { "B3-N3", "B3-N3", "B3-N3" } },
                                                 { "330000N", { "B3-N3", "B3-N3" } },
                                                 { "112233N", { "B1N1", "B1N1", "B2N2+", "B2N2-", "B3-N3", "B3-N3" } },
                                                 { "222333N",
                                                   { "B2N2+", "B2N2+", "B2N2-", "B3-N3", "B3-N3", "B3-N3" } },
                                                 { "223330N", { "B2N2+", "B2N2+", "B3-N3", "B3-N3", "B3-N3" } },
                                                 { "333300N", { "B3+N3", "B3-N3", "B3-N3", "B3-N3" } } };

extern const string jsonSlatDescription;
void CreateSlatGeometry();
} // namespace mch
} // namespace o2
#endif
