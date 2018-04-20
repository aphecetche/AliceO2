// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file   SlatGeometry.cxx
/// \brief  Implementation of the slat stations geometry
/// \author Florian Damas <florian.damas@cern.ch>
/// \date   22 mars 2018

#include <iostream>
#include "TGeoCompositeShape.h"
#include "TGeoManager.h"
#include "TGeoMedium.h"
#include "TGeoShape.h"
#include "TGeoTube.h"
#include "TGeoVolume.h"
#include "TMath.h"
#include "TROOT.h"
#include "TSystem.h"

#include <string>
#include <array>

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"

#include "MCHSimulation/Materials.h"

#include "MCHSimulation/SlatGeometry.h"

using namespace rapidjson;
using namespace std;

namespace o2
{
namespace mch
{

void CreatePCBs();

void CreateSlats();

void CreateSupportPanels();

void CreateHalfChambers();

//______________________________________________________________________________
void CreateSlatGeometry()
{
  /// Main function which build and place the slats and the half-chambers volumes
  /// This function must me called by the MCH detector class to build the slat stations geometry.

  // create the necessary media
  CreateSlatGeometryMaterials();

  materialManager().printMaterials();

  // create the right vertical spacer (identical for any slat)
  new TGeoVolume("Right vertical spacer",
                 new TGeoBBox(kVertSpacerLength / 2., kSlatPanelHeight / 2., kSpacerWidth / 2.),
                 assertMedium(Medium::Noryl));

  // create the different PCB types
  CreatePCBs();

  // create the support panels
  CreateSupportPanels();

  // create the different slat types
  CreateSlats();

  // create and place the half-chambers in the top volume
  CreateHalfChambers();
}

//______________________________________________________________________________
void CreatePCBs()
{
  /// Build the different PCB types

  /// A PCB is a pile-up of several material layers, from in to out : sensitive gas, pcb plate and insulation
  /// There are two types of pcb plates : a "bending" and a "non-bending" one. We build the PCB volume such that the
  /// bending side faces the IP (z>0). When placing the slat on the half-chambers, the builder grabs the rotation to
  /// apply from the JSON. By doing so, we make sure that we match the mapping convention

  // Define some necessary variables
  string bendName, nonbendName; // pcb plate names
  const float height = kPCBHeight;
  float gasLength, pcbLength, radius, curvRad, ypos, pcbShift, gasShift;
  int numb;

  cout << endl << "Creating " << kPcbTypes.size() << " types of PCBs" << endl;
  for (const auto& pcbType : kPcbTypes) { // loop over the PCB types of the array

    auto name = (const char*)pcbType.data();

    cout << "PCB type " << name << endl;
    auto pcb = new TGeoVolumeAssembly(name);

    gasShift = 0.;
    pcbShift = 0.;

    numb = pcbType[1] - '0'; // char -> int conversion

    // change the variables according to the PCB shape if necessary
    switch (pcbType.front()) {
      case 'R':                      // rounded
        numb = pcbType.back() - '0'; // char -> int conversion
        gasLength = (numb == 1) ? kR1PCBLength : kGasLength;
        pcbLength = (numb == 1) ? kR1PCBLength : kRoundedPCBLength;
        gasShift = -(gasLength - kGasLength);
        pcbShift = -(pcbLength - kPCBLength);

        bendName = Form("%sB", name);
        nonbendName = Form("%sN", name);
        break;
      case 'S': // shortened
        gasLength = kShortPCBLength;
        pcbLength = kShortPCBLength;
        bendName = Form("S2B%c", pcbType.back());
        nonbendName = Form("S2N%c", pcbType.back());
        break;
      default: // normal
        gasLength = kGasLength;
        pcbLength = kPCBLength;
        bendName = (numb == 3) ? pcbType.substr(0, 3) : pcbType.substr(0, 2);
        nonbendName = (numb == 3) ? pcbType.substr(3) : pcbType.substr(2);
    }

    // create the volume of each material (a box by default)
    // sensitive gas
    auto gas = new TGeoVolume(Form("%s gas", name),
                              new TGeoBBox(Form("%sGasBox", name), gasLength / 2., kGasHeight / 2., kGasWidth / 2.),
                              assertMedium(Medium::SlatGas));
    // bending pcb plate
    auto bend = new TGeoVolume(bendName.data(),
                               new TGeoBBox(Form("%sBendBox", name), pcbLength / 2., height / 2., kPCBWidth / 2.),
                               assertMedium(Medium::Copper));
    // non-bending pcb plate
    auto nonbend = new TGeoVolume(nonbendName.data(),
                                  new TGeoBBox(Form("%sNonBendBox", name), pcbLength / 2., height / 2., kPCBWidth / 2.),
                                  assertMedium(Medium::Copper));
    // insulating material (G10)
    auto insu = new TGeoVolume(Form("%s insulation", name),
                               new TGeoBBox(Form("%sInsuBox", name), pcbLength / 2., height / 2., kInsuWidth / 2.),
                               assertMedium(Medium::G10));

    // horizontal spacers (noryl)
    auto top = gGeoManager->MakeBox(Form("%s top spacer", name), assertMedium(Medium::Noryl), pcbLength / 2.,
                                    kHoriSpacerHeight / 2., kSpacerWidth / 2.);
    auto bottom = new TGeoVolume(
      Form("%s bottom spacer", name),
      new TGeoBBox(Form("%sBottomSpacerBox", name), pcbLength / 2., kHoriSpacerHeight / 2., kSpacerWidth / 2.),
      assertMedium(Medium::Noryl));

    // change the volume shape if we are creating a rounded PCB
    if (pcbType.front() == 'R') {
      // LHC beam pipe radius ("R3" -> it is a slat of a station 4 or 5)
      radius = (numb == 3) ? kRadSt45 : kRadSt3;
      // y position of the PCB center w.r.t the beam pipe shape
      switch (numb) {
        case 1:
          ypos = 0.; // central for "R1"
          break;
        case 2:
          ypos = kRoundedSlatYposSt3; // "R2" -> station 3
          break;
        default:
          ypos = kRoundedSlatYposSt45; // "R3" -> station 4 or 5
          break;
      }
      // compute the radius of curvature of the PCB we want to create
      curvRad = radius + kRoundedSpacerLength;

      // create the relative pipe position
      auto gasTrans = new TGeoTranslation(Form("GasHoleY%.1fShift", ypos),
                                          -kRoundedPCBLength + (gasLength + kVertSpacerLength) / 2., -ypos, 0.);
      gasTrans->RegisterYourself();
      auto pcbTrans = new TGeoTranslation(Form("PCBholeY%.1fShift", ypos),
                                          -kRoundedPCBLength + (pcbLength + kVertSpacerLength) / 2., -ypos, 0.);
      pcbTrans->RegisterYourself();
      auto bottomTrans = new TGeoTranslation(Form("BottomSpacerHoleY%.1fShift", ypos),
                                             -kRoundedPCBLength + (pcbLength + kVertSpacerLength) / 2.,
                                             -ypos + (kGasHeight + kHoriSpacerHeight) / 2., 0.);
      bottomTrans->RegisterYourself();

      // for each volume, create a hole and change the volume shape by extracting the pipe shape
      new TGeoTube(Form("GasHoleR%.1f", curvRad), 0., curvRad, kGasWidth / 2);
      gas->SetShape(new TGeoCompositeShape(Form("R%.1fY%.1fGasShape", curvRad, ypos),
                                           Form("%sGasBox-GasHoleR%.1f:GasHoleY%.1fShift", name, curvRad, ypos)));

      new TGeoTube(Form("HoleR%.1f", curvRad), 0., curvRad, kPCBWidth / 2.);
      bend->SetShape(new TGeoCompositeShape(Form("R%.1fY%.1fBendShape", curvRad, ypos),
                                            Form("%sBendBox-HoleR%.1f:PCBholeY%.1fShift", name, curvRad, ypos)));

      nonbend->SetShape(new TGeoCompositeShape(Form("R%.1fY%.1fNonBendShape", curvRad, ypos),
                                               Form("%sNonBendBox-HoleR%.1f:PCBholeY%.1fShift", name, curvRad, ypos)));

      new TGeoTube(Form("InsuHoleR%.1f", curvRad), 0., curvRad, kInsuWidth / 2.);
      insu->SetShape(new TGeoCompositeShape(Form("R%.1fY%.1fInsuShape", curvRad, ypos),
                                            Form("%sInsuBox-InsuHoleR%.1f:PCBholeY%.1fShift", name, curvRad, ypos)));

      if (pcbType.back() != '1') { // change the bottom spacer shape for "R2" and "R3" PCBs
        new TGeoTube(Form("BottomSpacerHoleR%.1f", radius), 0., radius, kSpacerWidth / 2);
        bottom->SetShape(new TGeoCompositeShape(
          Form("R%.1fY%.1fBottomSpacerShape", radius, ypos),
          Form("%sBottomSpacerBox-BottomSpacerHoleR%.1f:BottomSpacerHoleY%.1fShift", name, radius, ypos)));
      }
    }

    // place all the layers in the pcb volume assembly

    float width = kGasWidth; // increment this value when adding a new layer
    pcb->AddNode(gas, 1, new TGeoTranslation(gasShift / 2., 0., 0.));

    width += kPCBWidth;
    pcb->AddNode(bend, 1, new TGeoTranslation(pcbShift / 2., 0., width / 2.));
    pcb->AddNode(nonbend, 2, new TGeoTranslation(pcbShift / 2., 0., -width / 2.));

    width += kInsuWidth;
    pcb->AddNode(insu, 1, new TGeoTranslation(pcbShift / 2., 0., width / 2.));
    pcb->AddNode(insu, 2, new TGeoTranslation(pcbShift / 2., 0., -width / 2.));

    pcb->AddNode(top, 1, new TGeoTranslation(pcbShift / 2., (kGasHeight + kHoriSpacerHeight) / 2., 0.));
    pcb->AddNode(bottom, 1, new TGeoTranslation(pcbShift / 2., -(kGasHeight + kHoriSpacerHeight) / 2., 0.));
  }
}

//______________________________________________________________________________
void CreateSlats()
{
  /// Slat building function
  /// The different PCB types must have been built before calling this function !!!

  cout << endl << "Creating " << kSlatTypes.size() << " types of slat" << endl;

  for (const auto& slatType : kSlatTypes) {

    const string typeName = slatType.first;   // slat type name
    auto name = (const char*)typeName.data(); // slat name (easier to name volumes)
    const auto PCBs = slatType.second;        // PCB names vector
    const int nPCBs = PCBs.size();

    cout << "Slat " << name << " which has " << nPCBs << " PCBs" << endl;

    // create the slat volume assembly
    auto slat = new TGeoVolumeAssembly(name);
    const float height = kSlatPanelHeight;
    float length = 2 * kVertSpacerLength; // vertical spacers

    float panelShift = 0;

    // compute the slat center
    const float center = (nPCBs - 1) * kGasLength / 2;

    float PCBlength, gasLength;
    int ivol = 0;
    // loop over the number of PCBs in the current slat
    for (const auto& pcb : PCBs) {

      gasLength = kGasLength;

      switch (pcb.front()) {
        case 'R':
          PCBlength = (pcb.back() == '1') ? kR1PCBLength : kRoundedPCBLength;
          panelShift -= PCBlength - kRoundedPCBLength;
          break;
        case 'S':
          PCBlength = kShortPCBLength;
          gasLength = kShortPCBLength;
          panelShift += PCBlength - kPCBLength;
          break;
        default:
          PCBlength = kPCBLength;
      }

      length += PCBlength;

      // place the corresponding PCB volume in the slat and correct the origin of the slat
      slat->AddNode(gGeoManager->GetVolume(pcb.data()), ivol + 1,
                    new TGeoTranslation(ivol * kPCBLength - 0.5 * (kPCBLength - gasLength) - center, 0, 0));
      ivol++;

    } // end of the PCBs loop

    float leftSpacerHeight = kSlatPanelHeight;
    if (typeName.find('R') < typeName.size()) {
      length -= kVertSpacerLength;   // don't count the vertical spacer length twice in the case of rounded slat
      leftSpacerHeight = kGasHeight; // to avoid overlaps with the horizontal spacers
    }

    // left vertical spacer
    auto left = new TGeoVolume(
      Form("%s left vertical spacer", name),
      new TGeoBBox(Form("%sLeftSpacerBox", name), kVertSpacerLength / 2., leftSpacerHeight / 2., kSpacerWidth / 2.),
      assertMedium(Medium::Noryl));

    // glue a slat panel on each side of the PCBs
    auto panel = new TGeoVolumeAssembly(Form("%s panel", name));

    // glue
    auto glue = new TGeoVolume(Form("%s panel glue", name),
                               new TGeoBBox(Form("%sGlueBox", name), length / 2., height / 2., kGlueWidth / 2.),
                               assertMedium(Medium::Glue));

    // nomex (bulk)
    auto nomexBulk =
      new TGeoVolume(Form("%s panel nomex (bulk)", name),
                     new TGeoBBox(Form("%sNomexBulkBox", name), length / 2., height / 2., kNomexBulkWidth / 2.),
                     assertMedium(Medium::NomexBulk));

    // carbon fiber
    auto carbon = new TGeoVolume(Form("%s panel carbon fiber", name),
                                 new TGeoBBox(Form("%sCarbonBox", name), length / 2., height / 2., kCarbonWidth / 2.),
                                 assertMedium(Medium::Carbon));
    // nomex (honeycomb)
    auto nomex = new TGeoVolume(Form("%s panel nomex (honeycomb)", name),
                                new TGeoBBox(Form("%sNomexBox", name), length / 2., height / 2., kNomexWidth / 2.),
                                assertMedium(Medium::Nomex));

    // change the volume shape if we are creating a rounded slat
    if (typeName.find('R') < typeName.size()) {

      // LHC beam pipe radius ("NR3" -> it is a slat of a station 4 or 5)
      const float radius = (typeName.back() == '3') ? kRadSt45 : kRadSt3;

      // extreme angle values for the rounded spacer
      double angMin;
      double angMax = 90.;

      // position of the slat center w.r.t the beam pipe center
      float ypos;
      float xpos = (length - kVertSpacerLength) / 2.;
      float xRoundedPos = xpos - panelShift / 2.;
      switch (typeName.back()) { // get the last character
        case '1':                // central for "S(N)R1"
          ypos = 0.;
          xpos = 2 * kPCBLength + (panelShift + kVertSpacerLength) / 2.;
          xRoundedPos = kRoundedPCBLength + kPCBLength - kRoundedSpacerLength / 2.;
          angMin =
            -TMath::RadToDeg() * TMath::ACos((kRoundedPCBLength - kR1PCBLength - kRoundedSpacerLength / 2.) / radius);
          angMax = -angMin;
          break;
        case '2': // "S(N)R2" -> station 3
          ypos = kRoundedSlatYposSt3;
          angMin =
            TMath::RadToDeg() * TMath::ASin((ypos - kSlatPanelHeight / 2.) / (radius + kRoundedSpacerLength / 2.));
          break;
        default: // "NR3" -> station 4 or 5
          ypos = kRoundedSlatYposSt45;
          angMin =
            TMath::RadToDeg() * TMath::ASin((ypos - kSlatPanelHeight / 2.) / (radius + kRoundedSpacerLength / 2.));
          break;
      }

      // create the rounded spacer
      auto rounded = gGeoManager->MakeTubs(Form("%s rounded spacer", name), assertMedium(Medium::Noryl), radius,
                                           radius + kRoundedSpacerLength, kSpacerWidth, angMin, angMax);
      slat->AddNode(rounded, 1, new TGeoTranslation(-xRoundedPos, -ypos, 0.));

      // create the pipe position
      auto pipeShift = new TGeoTranslation(Form("%sPanelHoleShift", name), -xpos, -ypos, 0.);
      pipeShift->RegisterYourself();

      auto leftSpacerShift = new TGeoTranslation(Form("%sLeftSpacerShift", name), 0., -ypos, 0.);
      leftSpacerShift->RegisterYourself();

      // for each volume, create a hole and change the volume shape by extracting the pipe shape

      new TGeoTube(Form("%sGlueHole", name), 0., radius, kGlueWidth / 2.);
      glue->SetShape(new TGeoCompositeShape(Form("%sGlueShape", name),
                                            Form("%sGlueBox-%sGlueHole:%sPanelHoleShift", name, name, name)));

      new TGeoTube(Form("%sNomexBulkHole", name), 0., radius, kNomexBulkWidth / 2.);
      nomexBulk->SetShape(new TGeoCompositeShape(
        Form("%sNomexBulkShape", name), Form("%sNomexBulkBox-%sNomexBulkHole:%sPanelHoleShift", name, name, name)));

      new TGeoTube(Form("%sCarbonHole", name), 0., radius, kCarbonWidth / 2.);
      carbon->SetShape(new TGeoCompositeShape(Form("%sCarbonShape", name),
                                              Form("%sCarbonBox-%sCarbonHole:%sPanelHoleShift", name, name, name)));

      new TGeoTube(Form("%sNomexHole", name), 0., radius, kNomexWidth / 2.);
      nomex->SetShape(new TGeoCompositeShape(Form("%sNomexShape", name),
                                             Form("%sNomexBox-%sNomexHole:%sPanelHoleShift", name, name, name)));

      new TGeoTube(Form("%sLeftSpacerHole", name), 0., radius, kSpacerWidth / 2.);
      left->SetShape(new TGeoCompositeShape(
        Form("%sLeftSpacerShape", name), Form("%sLeftSpacerBox-%sLeftSpacerHole:%sLeftSpacerShift", name, name, name)));
    } // end of the "rounded" condition

    // place all the layers in the slat panel volume assembly
    // be careful : the panel origin is on the glue edge !

    float width = kGlueWidth; // increment this value when adding a new layer
    panel->AddNode(glue, 1, new TGeoTranslation(0., 0., width / 2.));

    panel->AddNode(nomexBulk, 1, new TGeoTranslation(0., 0., width + kNomexBulkWidth / 2.));
    width += kNomexBulkWidth;

    panel->AddNode(glue, 2, new TGeoTranslation(0., 0., width + kGlueWidth / 2.));
    width += kGlueWidth;

    panel->AddNode(carbon, 1, new TGeoTranslation(0., 0., width + kCarbonWidth / 2.));
    width += kCarbonWidth;

    panel->AddNode(nomex, 1, new TGeoTranslation(0., 0., width + kNomexWidth / 2.));
    width += kNomexWidth;

    panel->AddNode(carbon, 2, new TGeoTranslation(0., 0., width + kCarbonWidth / 2.));
    width += kCarbonWidth;

    // place the panel volume on each side of the slat volume assembly
    slat->AddNode(panel, 1, new TGeoTranslation(panelShift / 2., 0., kTotalPCBWidth / 2.));
    auto mirror = new TGeoRotation();
    mirror->ReflectZ(true);
    slat->AddNode(panel, 2, new TGeoCombiTrans(panelShift / 2., 0., -kTotalPCBWidth / 2., mirror));

    // place the vertical spacers
    slat->AddNode(gGeoManager->GetVolume("Right vertical spacer"), 1,
                  new TGeoTranslation((length - kVertSpacerLength + panelShift) / 2., 0., 0.));
    // don't place a left spacer for S(N)R1 slat
    if (typeName.back() != '1')
      slat->AddNode(left, 1, new TGeoTranslation(-(length - kVertSpacerLength - panelShift) / 2., 0., 0.));

  } // end of the slat loop
}

//______________________________________________________________________________
void CreateSupportPanels()
{
  /// Function building the half-chamber support panels (one different per chamber)

  for (int i = 5; i <= 10; i++) {

    // define the support panel volume
    auto support = new TGeoVolumeAssembly(Form("Ch%dSupportPanel", i));

    float length, height;
    if (i <= 6) { // station 3 half-chambers
      height = kSupportHeightSt3;
      length = (i == 5) ? kSupportLengthCh5 : kSupportLengthCh6;
    } else { // station 4 or 5
      length = kSupportLengthSt45;
      height = (i <= 8) ? kSupportHeightSt4 : kSupportHeightSt5;
    }

    // LHC beam pipe radius at the given chamber z position
    const float radius = (i <= 6) ? kRadSt3 : kRadSt45;

    float width = 0.; // increment this value when adding a new layer

    cout << "Support panel for the chamber " << i << " : radius = " << radius << ", length = " << length
         << ", height = " << height << endl;
    // create the hole in the nomex volume
    auto nomexHole = new TGeoTube(Form("NomexSupportPanelHoleCh%d", i), 0., radius, kNomexSupportWidth / 2.);

    // place this shape on the ALICE x=0 coordinate
    auto holeTrans = new TGeoTranslation(Form("holeCh%dShift", i), (-length + kVertSpacerLength) / 2., 0., 0.);
    holeTrans->RegisterYourself();

    // create a box for the nomex volume
    auto nomexBox =
      new TGeoBBox(Form("NomexSupportPanelCh%dBox", i), length / 2., height / 2., kNomexSupportWidth / 2.);

    // change the nomex volume shape by extracting the pipe shape
    auto nomexShape =
      new TGeoCompositeShape(Form("NomexSupportPanelCh%dShape", i),
                             Form("NomexSupportPanelCh%dBox-NomexSupportPanelHoleCh%d:holeCh%dShift", i, i, i));

    // create the nomex volume and place it in the support panel
    width += kNomexSupportWidth;
    support->AddNode(new TGeoVolume(Form("NomexSupportPanelCh%d", i), nomexShape, assertMedium(Medium::Nomex)), i,
                     new TGeoTranslation(length / 2., 0., 0.));

    // create the hole in the glue volume
    auto glueHole = new TGeoTube(Form("GlueSupportPanelHoleCh%d", i), 0., radius, kGlueSupportWidth / 2.);

    // create a box for the glue volume
    auto glueBox = new TGeoBBox(Form("GlueSupportPanelCh%dBox", i), length / 2., height / 2., kGlueSupportWidth / 2.);

    // change the glue volume shape by extracting the pipe shape
    auto glueShape =
      new TGeoCompositeShape(Form("GlueSupportPanelCh%dShape", i),
                             Form("GlueSupportPanelCh%dBox-GlueSupportPanelHoleCh%d:holeCh%dShift", i, i, i));

    // create the glue volume
    auto glue = new TGeoVolume(Form("GlueSupportPanelCh%d", i), glueShape, assertMedium(Medium::Glue));

    // place it on each side of the nomex volume
    width += kGlueSupportWidth;
    support->AddNode(glue, 1, new TGeoTranslation(length / 2., 0., width / 2.));
    support->AddNode(glue, 2, new TGeoTranslation(length / 2., 0., -width / 2.));

    // create the hole in the carbon volume
    auto carbonHole = new TGeoTube(Form("CarbonSupportPanelHoleCh%d", i), 0., radius, kCarbonSupportWidth / 2.);

    // create a box for the carbon volume
    auto carbonBox =
      new TGeoBBox(Form("CarbonSupportPanelCh%dBox", i), length / 2., height / 2., kCarbonSupportWidth / 2.);

    // change the carbon volume shape by extracting the pipe shape
    auto carbonShape =
      new TGeoCompositeShape(Form("CarbonSupportPanelCh%dShape", i),
                             Form("CarbonSupportPanelCh%dBox-CarbonSupportPanelHoleCh%d:holeCh%dShift", i, i, i));

    // create the carbon volume
    auto carbon = new TGeoVolume(Form("CarbonSupportPanelCh%d", i), carbonShape, assertMedium(Medium::Carbon));

    // place it on each side of the glue volume
    width += kCarbonSupportWidth;
    support->AddNode(carbon, 1, new TGeoTranslation(length / 2., 0., width / 2.));
    support->AddNode(carbon, 2, new TGeoTranslation(length / 2., 0., -width / 2.));

  } // end of the chamber loop
}

//______________________________________________________________________________
void CreateHalfChambers()
{
  /// Build the slat half-chambers
  /// The different slat types must have been built before calling this function !!!

  // read the json containing all the necessary parameters to place the slat volumes in the half-chambers
  StringStream is(jsonSlatDescription.c_str());

  Document doc;
  doc.ParseStream(is);

  // get the "half-chambers" array
  Value& hChs = doc["HalfChambers"];
  assert(hChs.IsArray());

  // loop over the objects (half-chambers) of the array
  for (const auto& halfCh : hChs.GetArray()) {
    // check that "halfCh" is an object
    if (!halfCh.IsObject())
      throw runtime_error("Can't create the half-chambers : wrong Value input");

    const auto moduleID = halfCh["moduleID"].GetInt();
    const string name = halfCh["name"].GetString();
    // get the chamber number (if the chamber name has a '0' at the 3rd digit, take the number after; otherwise it's the
    // chamber 10)
    const int nCh = (name.find('0') == 2) ? name[3] - '0' /* char -> int conversion */ : 10;

    cout << endl << "Creating half-chamber " << name << endl;
    auto halfChVol = new TGeoVolumeAssembly(name.data());

    // place the support panel corresponding to the chamber number
    auto supRot = new TGeoRotation();
    if (moduleID % 2)
      supRot->RotateY(180.);
    halfChVol->AddNode(gGeoManager->GetVolume(Form("Ch%dSupportPanel", nCh)), moduleID, supRot);

    // place the slat volumes on the different nodes of the half-chamber
    for (const auto& slat : halfCh["nodes"].GetArray()) {
      // check that "slat" is an object
      if (!slat.IsObject())
        throw runtime_error("Can't create the slat : wrong Value input");

      const auto detID = slat["detID"].GetInt();

      cout << "Placing the slat " << detID << " of type " << slat["type"].GetString() << endl;

      // create the slat rotation matrix
      auto slatRot = new TGeoRotation(Form("Slat%drotation", detID), slat["rotation"][0].GetDouble(),
                                      slat["rotation"][1].GetDouble(), slat["rotation"][2].GetDouble(),
                                      slat["rotation"][3].GetDouble(), slat["rotation"][4].GetDouble(),
                                      slat["rotation"][5].GetDouble());

      // place the slat on the half-chamber volume
      halfChVol->AddNode(gGeoManager->GetVolume(slat["type"].GetString()), detID,
                         new TGeoCombiTrans(Form("Slat%dposition", detID), slat["position"][0].GetDouble(),
                                            slat["position"][1].GetDouble(), slat["position"][2].GetDouble(), slatRot));

    } // end of the node loop
    cout << halfCh["nodes"].Size() << " slats placed on the half-chamber " << halfCh["name"].GetString() << endl;

    // place the half-chamber in the top volume
    auto halfChRot = new TGeoRotation(Form("%srotation", name.data()), halfCh["rotation"][0].GetDouble(),
                                      halfCh["rotation"][1].GetDouble(), halfCh["rotation"][2].GetDouble(),
                                      halfCh["rotation"][3].GetDouble(), halfCh["rotation"][4].GetDouble(),
                                      halfCh["rotation"][5].GetDouble());

    gGeoManager->GetTopVolume()->AddNode(
      halfChVol, moduleID,
      new TGeoCombiTrans(Form("HalfCh%dposition", moduleID), halfCh["position"][0].GetDouble(),
                         halfCh["position"][1].GetDouble(), halfCh["position"][2].GetDouble(), halfChRot));

    // if the dipole is present in the geometry, we place the station 3 half-chambers in it (actually not working)
    if (gGeoManager->GetVolume("Dipole") && (nCh == 5 || nCh == 6)) {
      gGeoManager->GetTopVolume()
        ->GetNode(Form("%s_%d", name.data(), moduleID))
        ->SetMotherVolume(gGeoManager->GetVolume("DDIP"));
      cout << endl << "Placing " << name << " in the Dipole" << endl;
      gGeoManager->GetTopVolume()->GetNode(Form("%s_%d", name.data(), moduleID))->Print();
    }
  } // end of the half-chambers loop

  cout << endl << hChs.Size() << " half-chambers placed on the top volume" << endl << endl;
}

//______________________________________________________________________________

/// Json string describing all the necessary parameters to place the slats in the half-chambers
const string jsonSlatDescription =
  R"(
{
  "HalfChambers": [
    {
      "name":"SC05I",
      "moduleID":4,
      "position":[0.00, -0.1663, -959.75],
      "rotation":[90, 0, 90.794, 90, 0.794, 90],
      "nodes":[
        {
          "detID":500,
          "type":"122000SR1",
          "position":[81.25, 0.00, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":501,
          "type":"112200SR2",
          "position":[81.25, 37.80, -4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":502,
          "type":"122200S",
          "position":[81.25, 75.50, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":503,
          "type":"222000N",
          "position":[61.25, 112.80, -4.00],
          "rotation":[90, 0, 90, 270, 180, 0]
        },
        {
          "detID":504,
          "type":"220000N",
          "position":[41.25, 146.50, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":514,
          "type":"220000N",
          "position":[41.25, -146.50, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":515,
          "type":"222000N",
          "position":[61.25, -112.80, -4.00],
          "rotation":[90, 0, 90, 270, 180, 0]
        },
        {
          "detID":516,
          "type":"122200S",
          "position":[81.25, -75.50, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":517,
          "type":"112200SR2",
          "position":[81.25, -37.80, -4.00],
          "rotation":[90, 0, 90, 270, 180, 0]
        }
      ]
    },

    {
      "name":"SC05O",
      "moduleID":5,
      "position":[0.00, 0.1663, -975.25],
      "rotation":[90, 0, 90.794, 90, 0.794, 90],
      "nodes":[
        {
          "detID":505,
          "type":"220000N",
          "position":[-41.25, 146.50, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":506,
          "type":"222000N",
          "position":[-61.25, 112.80, 4.00],
          "rotation":[90, 180, 90, 270, 0, 0]
        },
        {
          "detID":507,
          "type":"122200S",
          "position":[-81.25, 75.50, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":508,
          "type":"112200SR2",
          "position":[-81.25, 37.80, 4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":509,
          "type":"122000SR1",
          "position":[-81.25, 0.00, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":510,
          "type":"112200SR2",
          "position":[-81.25, -37.80, 4.00],
          "rotation":[90, 180, 90, 270, 0, 0]
        },
        {
          "detID":511,
          "type":"122200S",
          "position":[-81.25, -75.50, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":512,
          "type":"222000N",
          "position":[-61.25, -112.80, 4.00],
          "rotation":[90, 180, 90, 270, 0, 0]
        },
        {
          "detID":513,
          "type":"220000N",
          "position":[-41.25, -146.50, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        }
      ]
    },

    {
      "name":"SC06I",
      "moduleID":6,
      "position":[0.00, -0.1663, -990.75],
      "rotation":[90, 0, 90.794, 90, 0.794, 90],
      "nodes":[
        {
          "detID":600,
          "type":"122000NR1",
          "position":[81.25, 0.00, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":601,
          "type":"112200NR2",
          "position":[81.25, 37.80, -4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":602,
          "type":"122200N",
          "position":[81.25, 75.50, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":603,
          "type":"222000N",
          "position":[61.25, 112.80, -4.00],
          "rotation":[90, 0, 90, 270, 180, 0]
        },
        {
          "detID":604,
          "type":"220000N",
          "position":[41.25, 146.50, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":614,
          "type":"220000N",
          "position":[41.25, -146.50, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":615,
          "type":"222000N",
          "position":[61.25, -112.80, -4.00],
          "rotation":[90, 0, 90, 270, 180, 0]
        },
        {
          "detID":616,
          "type":"122200N",
          "position":[81.25, -75.50, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":617,
          "type":"112200NR2",
          "position":[81.25, -37.80, -4.00],
          "rotation":[90, 0, 90, 270, 180, 0]
        }
      ]
    },

    {
      "name":"SC06O",
      "moduleID":7,
      "position":[0.00, 0.1663, -1006.25],
      "rotation":[90, 0, 90.794, 90, 0.794, 90],
      "nodes":[
        {
          "detID":605,
          "type":"220000N",
          "position":[-41.25, 146.50, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":606,
          "type":"222000N",
          "position":[-61.25, 112.80, 4.00],
          "rotation":[90, 180, 90, 270, 0, 0]
        },
        {
          "detID":607,
          "type":"122200N",
          "position":[-81.25, 75.50, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":608,
          "type":"112200NR2",
          "position":[-81.25, 37.80, 4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":609,
          "type":"122000NR1",
          "position":[-81.25, 0.00, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":610,
          "type":"112200NR2",
          "position":[-81.25, -37.80, 4.00],
          "rotation":[90, 180, 90, 270, 0, 0]
        },
        {
          "detID":611,
          "type":"122200N",
          "position":[-81.25, -75.5, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":612,
          "type":"222000N",
          "position":[-61.25, -112.80, 4.00],
          "rotation":[90, 180, 90, 270, 0, 0]
        },
        {
          "detID":613,
          "type":"220000N",
          "position":[-41.25, -146.50, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        }
      ]
    },

    {
      "name":"SC07I",
      "moduleID":8,
      "position":[0.00, -0.1663, -1259.75],
      "rotation":[90, 0, 90.794, 90, 0.794, 90],
      "nodes":[
        {
          "detID":700,
          "type":"122330N",
          "position":[140.00, 0.00, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":701,
          "type":"112233NR3",
          "position":[121.25, 38.20, -4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":702,
          "type":"112230N",
          "position":[101.25, 72.60, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":703,
          "type":"222330N",
          "position":[101.25, 109.20, -4.00],
          "rotation":[90, 0, 90, 270, 180, 0]
        },
        {
          "detID":704,
          "type":"223300N",
          "position":[81.25, 138.50, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":705,
          "type":"333000N",
          "position":[61.25, 175.50, -4.00],
          "rotation":[90, 0, 90, 270, 180, 0]
        },
        {
          "detID":706,
          "type":"330000N",
          "position":[41.25, 204.50, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":720,
          "type":"330000N",
          "position":[41.25, -204.50, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":721,
          "type":"333000N",
          "position":[61.25, -175.50, -4.00],
          "rotation":[90, 0, 90, 270, 180, 0]
        },
        {
          "detID":722,
          "type":"223300N",
          "position":[81.25, -138.50, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":723,
          "type":"222330N",
          "position":[101.25, -109.20, -4.00],
          "rotation":[90, 0, 90, 270, 180, 0]
        },
        {
          "detID":724,
          "type":"112230N",
          "position":[101.25, -72.60, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":725,
          "type":"112233NR3",
          "position":[121.25, -38.20, -4.00],
          "rotation":[90, 0, 90, 270, 180, 0]
        }
      ]
    },

    {
      "name":"SC07O",
      "moduleID":9,
      "position":[0.00, -0.1663, -1284.25],
      "rotation":[90, 0, 90.794, 90, 0.794, 90],
      "nodes":[
        {
          "detID":707,
          "type":"330000N",
          "position":[-41.25, 204.5, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":708,
          "type":"333000N",
          "position":[-61.25, 175.50, 4.00],
          "rotation":[90, 180, 90, 270, 0, 0]
        },
        {
          "detID":709,
          "type":"223300N",
          "position":[-81.25, 138.50, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":710,
          "type":"222330N",
          "position":[-101.25, 109.20, 4.00],
          "rotation":[90, 180, 90, 270, 0, 0]
        },
        {
          "detID":711,
          "type":"112230N",
          "position":[-101.25, 72.60, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":712,
          "type":"112233NR3",
          "position":[-121.25, 38.20, 4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":713,
          "type":"122330N",
          "position":[-140.00, 0.00, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":714,
          "type":"112233NR3",
          "position":[-121.25, -38.20, 4.00],
          "rotation":[90, 180, 90, 270, 0, 0]
        },
        {
          "detID":715,
          "type":"112230N",
          "position":[-101.25, -72.60, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":716,
          "type":"222330N",
          "position":[-101.25, -109.20, 4.00],
          "rotation":[90, 180, 90, 270, 0, 0]
        },
        {
          "detID":717,
          "type":"223300N",
          "position":[-81.25, -138.50, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":718,
          "type":"333000N",
          "position":[-61.25, -175.50, 4.00],
          "rotation":[90, 180, 90, 270, 0, 0]
        },
        {
          "detID":719,
          "type":"330000N",
          "position":[-41.25, -204.50, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        }
      ]
    },

    {
      "name":"SC08I",
      "moduleID":10,
      "position":[0.00, -0.1663, -1299.75],
      "rotation":[90, 0, 90.794, 90, 0.794, 90],
      "nodes":[
        {
          "detID":800,
          "type":"122330N",
          "position":[140.00, 0.00, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":801,
          "type":"112233NR3",
          "position":[121.25, 38.20, -4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":802,
          "type":"112230N",
          "position":[101.25, 76.05, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":803,
          "type":"222330N",
          "position":[101.25, 113.60, -4.00],
          "rotation":[90, 0, 90, 270, 180, 0]
        },
        {
          "detID":804,
          "type":"223300N",
          "position":[81.25, 143.00, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":805,
          "type":"333000N",
          "position":[61.25, 180.00, -4.00],
          "rotation":[90, 0, 90, 270, 180, 0]
        },
        {
          "detID":806,
          "type":"330000N",
          "position":[41.25, 208.60, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":820,
          "type":"330000N",
          "position":[41.25, -208.60, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":821,
          "type":"333000N",
          "position":[61.25, -180.00, -4.00],
          "rotation":[90, 0, 90, 270, 180, 0]
        },
        {
          "detID":822,
          "type":"223300N",
          "position":[81.25, -143.00, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":823,
          "type":"222330N",
          "position":[101.25, -113.60, -4.00],
          "rotation":[90, 0, 90, 270, 180, 0]
        },
        {
          "detID":824,
          "type":"112230N",
          "position":[101.25, -76.05, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":825,
          "type":"112233NR3",
          "position":[121.25, -38.20, -4.00],
          "rotation":[90, 0, 90, 270, 180, 0]
        }
      ]
    },

    {
      "name":"SC08O",
      "moduleID":11,
      "position":[0.00, -0.1663, -1315.25],
      "rotation":[90, 0, 90.794, 90, 0.794, 90],
      "nodes":[
        {
          "detID":807,
          "type":"330000N",
          "position":[-41.25, 208.60, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":808,
          "type":"333000N",
          "position":[-61.25, 180.00, 4.00],
          "rotation":[90, 180, 90, 270, 0, 0]
        },
        {
          "detID":809,
          "type":"223300N",
          "position":[-81.25, 143.00, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":810,
          "type":"222330N",
          "position":[-101.25, 113.60, 4.00],
          "rotation":[90, 180, 90, 270, 0, 0]
        },
        {
          "detID":811,
          "type":"112230N",
          "position":[-101.25, 76.05, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":812,
          "type":"112233NR3",
          "position":[-121.25, 38.20, 4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":813,
          "type":"122330N",
          "position":[-140.00, 0.00, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":814,
          "type":"112233NR3",
          "position":[-121.25, -38.20, 4.00],
          "rotation":[90, 180, 90, 270, 0, 0]
        },
        {
          "detID":815,
          "type":"112230N",
          "position":[-101.25, -76.05, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":816,
          "type":"222330N",
          "position":[-101.25, -113.60, 4.00],
          "rotation":[90, 180, 90, 270, 0, 0]
        },
        {
          "detID":817,
          "type":"223300N",
          "position":[-81.25, -143.00, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":818,
          "type":"333000N",
          "position":[-61.25, -180.00, 4.00],
          "rotation":[90, 180, 90, 270, 0, 0]
        },
        {
          "detID":819,
          "type":"330000N",
          "position":[-41.25, -208.60, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        }
      ]
    },

    {
      "name":"SC09I",
      "moduleID":12,
      "position":[0.00, -0.1663, -1398.85],
      "rotation":[90, 0, 90.794, 90, 0.794, 90],
      "nodes":[
        {
          "detID":900,
          "type":"122330N",
          "position":[140.00, 0.00, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":901,
          "type":"112233NR3",
          "position":[121.25, 38.20, -4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":902,
          "type":"112233N",
          "position":[121.25, 76.10, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":903,
          "type":"222333N",
          "position":[121.25, 113.70, -4.00],
          "rotation":[90, 0, 90, 270, 180, 0]
        },
        {
          "detID":904,
          "type":"223330N",
          "position":[101.25, 151.00, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":905,
          "type":"333300N",
          "position":[81.25, 188.05, -4.00],
          "rotation":[90, 0, 90, 270, 180, 0]
        },
        {
          "detID":906,
          "type":"333000N",
          "position":[61.25, 224.80, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":920,
          "type":"333000N",
          "position":[61.25, -224.80, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":921,
          "type":"333300N",
          "position":[81.25, -188.05, -4.00],
          "rotation":[90, 0, 90, 270, 180, 0]
        },
        {
          "detID":922,
          "type":"223330N",
          "position":[101.25, -151.00, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":923,
          "type":"222333N",
          "position":[121.25, -113.70, -4.00],
          "rotation":[90, 0, 90, 270, 180, 0]
        },
        {
          "detID":924,
          "type":"112233N",
          "position":[121.25, -76.10, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":925,
          "type":"112233NR3",
          "position":[121.25, -38.20, -4.00],
          "rotation":[90, 0, 90, 270, 180, 0]
        }
      ]
    },

    {
      "name":"SC09O",
      "moduleID":13,
      "position":[0.00, -0.1663, -1414.35],
      "rotation":[90, 0, 90.794, 90, 0.794, 90],
      "nodes":[
        {
          "detID":907,
          "type":"333000N",
          "position":[-61.25, 224.80, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":908,
          "type":"333300N",
          "position":[-81.25, 188.05, 4.00],
          "rotation":[90, 180, 90, 270, 0, 0]
        },
        {
          "detID":909,
          "type":"223330N",
          "position":[-101.25, 151.00, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":910,
          "type":"222333N",
          "position":[-121.25, 113.70, 4.00],
          "rotation":[90, 180, 90, 270, 0, 0]
        },
        {
          "detID":911,
          "type":"112233N",
          "position":[-121.25, 76.10, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":912,
          "type":"112233NR3",
          "position":[-121.25, 38.20, 4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":913,
          "type":"122330N",
          "position":[-140.00, 0.00, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":914,
          "type":"112233NR3",
          "position":[-121.25, -38.20, 4.00],
          "rotation":[90, 180, 90, 270, 0, 0]
        },
        {
          "detID":915,
          "type":"112233N",
          "position":[-121.25, -76.10, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":916,
          "type":"222333N",
          "position":[-121.25, -113.70, 4.00],
          "rotation":[90, 180, 90, 270, 0, 0]
        },
        {
          "detID":917,
          "type":"223330N",
          "position":[-101.25, -151, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":918,
          "type":"333300N",
          "position":[-81.25, -188.05, 4.00],
          "rotation":[90, 180, 90, 270, 0, 0]
        },
        {
          "detID":919,
          "type":"333000N",
          "position":[-61.25, -224.80, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        }
      ]
    },

    {
      "name":"SC10I",
      "moduleID":14,
      "position":[0.00, -0.1663, -1429.85],
      "rotation":[90, 0, 90.794, 90, 0.794, 90],
      "nodes":[
        {
          "detID":1000,
          "type":"122330N",
          "position":[140.00, 0.00, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":1001,
          "type":"112233NR3",
          "position":[121.25, 38.20, -4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":1002,
          "type":"112233N",
          "position":[121.25, 76.10, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":1003,
          "type":"222333N",
          "position":[121.25, 113.70, -4.00],
          "rotation":[90, 0, 90, 270, 180, 0]
        },
        {
          "detID":1004,
          "type":"223330N",
          "position":[101.25, 151.00, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":1005,
          "type":"333300N",
          "position":[81.25, 188.05, -4.00],
          "rotation":[90, 0, 90, 270, 180, 0]
        },
        {
          "detID":1006,
          "type":"333000N",
          "position":[61.25, 224.80, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":1020,
          "type":"333000N",
          "position":[61.25, -224.80, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":1021,
          "type":"333300N",
          "position":[81.25, -188.05, -4.00],
          "rotation":[90, 0, 90, 270, 180, 0]
        },
        {
          "detID":1022,
          "type":"223330N",
          "position":[101.25, -151.00, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":1023,
          "type":"222333N",
          "position":[121.25, -113.70, -4.00],
          "rotation":[90, 0, 90, 270, 180, 0]
        },
        {
          "detID":1024,
          "type":"112233N",
          "position":[121.25, -76.10, 4.00],
          "rotation":[90, 0, 90, 90, 0, 0]
        },
        {
          "detID":1025,
          "type":"112233NR3",
          "position":[121.25, -38.20, -4.00],
          "rotation":[90, 0, 90, 270, 180, 0]
        }
      ]
    },

    {
      "name":"SC10O",
      "moduleID":15,
      "position":[0.00, -0.1663, -1445.35],
      "rotation":[90, 0, 90.794, 90, 0.794, 90],
      "nodes":[
        {
          "detID":1007,
          "type":"333000N",
          "position":[-61.25, 224.80, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":1008,
          "type":"333300N",
          "position":[-81.25, 188.05, 4.00],
          "rotation":[90, 180, 90, 270, 0, 0]
        },
        {
          "detID":1009,
          "type":"223330N",
          "position":[-101.25, 151.00, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":1010,
          "type":"222333N",
          "position":[-121.25, 113.70, 4.00],
          "rotation":[90, 180, 90, 270, 0, 0]
        },
        {
          "detID":1011,
          "type":"112233N",
          "position":[-121.25, 76.10, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":1012,
          "type":"112233NR3",
          "position":[-121.25, 38.20, 4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":1013,
          "type":"122330N",
          "position":[-140.00, 0.00, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":1014,
          "type":"112233NR3",
          "position":[-121.25, -38.20, 4.00],
          "rotation":[90, 180, 90, 270, 0, 0]
        },
        {
          "detID":1015,
          "type":"112233N",
          "position":[-121.25, -76.10, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":1016,
          "type":"222333N",
          "position":[-121.25, -113.70, 4.00],
          "rotation":[90, 180, 90, 270, 0, 0]
        },
        {
          "detID":1017,
          "type":"223330N",
          "position":[-101.25, -151.00, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        },
        {
          "detID":1018,
          "type":"333300N",
          "position":[-81.25, -188.05, 4.00],
          "rotation":[90, 180, 90, 270, 0, 0]
        },
        {
          "detID":1019,
          "type":"333000N",
          "position":[-61.25, -224.80, -4.00],
          "rotation":[90, 180, 90, 90, 180, 0]
        }
      ]
    }
  ]
}

)";
} // namespace mch
} // namespace o2
