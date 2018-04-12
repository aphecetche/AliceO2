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

void CreateNormalPCB(const string name);

void CreateRoundedPCB(const string name);

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

  // create the different PCB types
  cout << endl << "Creating " << kPcbTypes.size() << " types of PCBs" << endl;
  for (auto pcb : kPcbTypes) { // loop over the PCB types of the array

    // create a PCB volume assembly according to its shape
    switch (pcb.front()) { // get the first character of the pcb type name
      case 'R':
        CreateRoundedPCB(pcb);
        break;
      default:
        CreateNormalPCB(pcb);
        break;
    }
  }

  // create the support panels
  CreateSupportPanels();

  // create the different slat types
  CreateSlats();

  // create and place the half-chambers in the top volume
  CreateHalfChambers();
}

//______________________________________________________________________________
void CreateNormalPCB(const string name)
{
  /// Build a normal shape PCB (pcb plates sandwich with the sensitive gas inside for the moment)
  /// input : * the name of the PCB type ("B1N1","B2N2-","B2N2+","B3-N3", "B3+N3", "S2-" or "S2+")

  // check if the given name match this builder
  if (!(name.front() == 'B' || name.front() == 'S'))
    throw runtime_error("Normal PCB builder called with the wrong PCB type name");

  cout << "PCB type " << name << endl;

  // get the number from the PCB type name -> important for this builder
  const Int_t numb = name[1] - '0'; // char -> int conversion

  // get the pcb plate names and length
  string bendName = (numb == 3) ? name.substr(0, 3) : name.substr(0, 2);
  string nonbendName = (numb == 3) ? name.substr(3) : name.substr(2);
  Double_t pcbLength = kPCBLength;

  // correct these values for shortened PCB
  if (name.front() == 'S') {
    bendName = Form("S2B%c", name.back());
    nonbendName = Form("S2N%c", name.back());
    pcbLength = kShortPCBLength;
  }

  auto pcb = new TGeoVolumeAssembly(name.data());
  Double_t width = 0; // increment this value when adding a new layer

  // place the gas
  width += kGasWidth;
  pcb->AddNode(gGeoManager->MakeBox(Form("Gas of %s", name.data()), assertMedium(Medium::SlatGas), pcbLength / 2.,
                                    kGasHeight / 2., width / 2.),
               1);

  // the pcb plates (bending and non-bending)
  width += kPCBWidth;
  auto bend = gGeoManager->MakeBox(bendName.data(), assertMedium(Medium::Copper), pcbLength / 2., kPCBHeight / 2.,
                                   kPCBWidth / 2.);
  bend->SetLineColor(kRed); // visualisation help
  pcb->AddNode(bend, 1, new TGeoTranslation(0., 0., width / 2.));
  auto nonbend = gGeoManager->MakeBox(nonbendName.data(), assertMedium(Medium::Copper), pcbLength / 2., kPCBHeight / 2.,
                                      kPCBWidth / 2.);
  nonbend->SetLineColor(kGreen); // visualisation help
  pcb->AddNode(nonbend, 2, new TGeoTranslation(0., 0., -width / 2.));

  // the insulating material (G10)
  auto insu = gGeoManager->MakeBox(Form("Insulation of %s", name.data()), assertMedium(Medium::G10), pcbLength / 2.,
                                   kPCBHeight / 2., kInsuWidth / 2.);
  width += kInsuWidth;
  pcb->AddNode(insu, 1, new TGeoTranslation(0., 0., width / 2.));
  pcb->AddNode(insu, 2, new TGeoTranslation(0., 0., -width / 2.));

  // glue between insulation and a first layer of nomex (bulk)
  auto glue = gGeoManager->MakeBox(Form("Glue of %s", name.data()), assertMedium(Medium::Glue), pcbLength / 2.,
                                   kPCBHeight / 2., kGlueWidth / 2.);
  width += kGlueWidth;
  pcb->AddNode(glue, 1, new TGeoTranslation(0., 0., width / 2.));
  pcb->AddNode(glue, 2, new TGeoTranslation(0., 0., -width / 2.));

  // nomex bulk
  auto nomexBulk = gGeoManager->MakeBox(Form("Nomex bulk of %s", name.data()), assertMedium(Medium::NomexBulk),
                                        pcbLength / 2., kPCBHeight / 2., kNomexBulkWidth / 2.);
  width += kNomexBulkWidth;
  pcb->AddNode(nomexBulk, 1, new TGeoTranslation(0., 0., width / 2.));
  pcb->AddNode(nomexBulk, 2, new TGeoTranslation(0., 0., -width / 2.));

  // glue between nomex bulk and a first carbon fiber layer
  width += kGlueWidth;
  pcb->AddNode(glue, 3, new TGeoTranslation(0., 0., width / 2.));
  pcb->AddNode(glue, 4, new TGeoTranslation(0., 0., -width / 2.));

  // first carbon fiber layer
  auto carbon = gGeoManager->MakeBox(Form("Carbon fiber of %s", name.data()), assertMedium(Medium::Carbon),
                                     pcbLength / 2., kPCBHeight / 2., kCarbonWidth / 2.);
  width += kCarbonWidth;
  pcb->AddNode(carbon, 1, new TGeoTranslation(0., 0., width / 2.));
  pcb->AddNode(carbon, 2, new TGeoTranslation(0., 0., -width / 2.));

  // honeycomb nomex
  auto nomex = gGeoManager->MakeBox(Form("Nomex honeycomb of %s", name.data()), assertMedium(Medium::Nomex),
                                    pcbLength / 2., kPCBHeight / 2., kNomexWidth / 2.);
  width += kNomexWidth;
  pcb->AddNode(nomex, 1, new TGeoTranslation(0., 0., width / 2.));
  pcb->AddNode(nomex, 2, new TGeoTranslation(0., 0., -width / 2.));

  // second carbon fiber layer
  width += kCarbonWidth;
  pcb->AddNode(carbon, 3, new TGeoTranslation(0., 0., width / 2.));
  pcb->AddNode(carbon, 4, new TGeoTranslation(0., 0., -width / 2.));
}

//______________________________________________________________________________
void CreateRoundedPCB(const string name)
{
  /// Build a rounded shape PCB (pcb plates sandwich with the sensitive gas inside for the moment)
  /// input : * the name of the PCB type ("R1", "R2" or "R3")

  // check if the given name match this builder
  if (name.front() != 'R')
    throw runtime_error("Rounded PCB builder called with the wrong PCB type name");

  cout << "PCB type " << name << endl;

  // get the number from the PCB type name
  const Int_t numb = name.back() - '0'; // char -> int conversion

  // LHC beam pipe radius ("R3" -> it is a slat of a station 4 or 5)
  const Double_t radius = (numb == 3) ? kRadSt45 : kRadSt3;
  // compute the radius of curvature of the PCB we want to create
  const Double_t curvRad = radius + kVertFrameLength;

  // y position of the PCB center w.r.t the beam pipe shape
  Double_t ypos;
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

  auto pcb = new TGeoVolumeAssembly(name.data());

  // create the hole in the gas
  auto gasHole = new TGeoTubeSeg(Form("GasHoleR%.1f", curvRad), 0., curvRad, kGasWidth / 2., -90., 90.);

  // place this shape on the ALICE x=0; y=0 coordinates
  auto pipeShift = new TGeoTranslation(Form("holeY%.1fShift", ypos), -(kGasLength + kVertSpacerLength) / 2., -ypos, 0.);
  pipeShift->RegisterYourself();

  // create a classic shape gas box
  auto gasBox = new TGeoBBox("GasBox", kGasLength / 2., kGasHeight / 2., kGasWidth / 2.);

  // change the gas volume shape by extracting the pipe shape
  auto gasShape = new TGeoCompositeShape(Form("R%.1fY%.1fGasShape", curvRad, ypos),
                                         Form("GasBox-GasHoleR%.1f:holeY%.1fShift", curvRad, ypos));

  // create the new gas volume and place it in the PCB volume assembly
  pcb->AddNode(new TGeoVolume(Form("Gas of %s", name.data()), gasShape, assertMedium(Medium::SlatGas)), 1);

  // create the hole in the pcb plate volume
  auto pcbHole = new TGeoTubeSeg(Form("PcbHoleR%.1f", curvRad), 0., curvRad, kPCBWidth / 2., -90., 90.);

  // create a box volume for the pcb plate
  auto pcbBox = new TGeoBBox("PcbBox", kPCBLength / 2., kPCBHeight / 2., kPCBWidth / 2.);

  // change the pcb volume shape by extracting the pipe shape
  auto pcbShape = new TGeoCompositeShape(Form("R%.1fY%.1fPcbShape", curvRad, ypos),
                                         Form("PcbBox-PcbHoleR%.1f:holeY%.1fShift", curvRad, ypos));

  auto bend = new TGeoVolume(Form("%sB", name.data()), pcbShape, assertMedium(Medium::Copper));
  bend->SetLineColor(kRed);
  pcb->AddNode(bend, 2, new TGeoTranslation(0., 0., (kGasWidth + kPCBWidth) / 2.));

  auto nonbend = new TGeoVolume(Form("%sN", name.data()), pcbShape, assertMedium(Medium::Copper));
  nonbend->SetLineColor(kGreen);
  pcb->AddNode(nonbend, 3, new TGeoTranslation(0., 0., -(kGasWidth + kPCBWidth) / 2.));
}

//______________________________________________________________________________
void CreateSlats()
{
  /// Slat building function

  cout << endl << "Creating " << kSlatTypes.size() << " types of slat" << endl;

  for (auto slat : kSlatTypes) {

    const string name = slat.first; // slat name
    auto PCBs = slat.second;        // PCB names vector
    const Int_t nPCBs = PCBs.size();

    cout << "Slat " << name << " which has " << nPCBs << " PCBs" << endl;

    // create the slat volume assembly
    auto slatVol = new TGeoVolumeAssembly(name.data());

    // compute the slat center
    const Double_t center = (nPCBs % 2) ? (nPCBs / 2) * kGasLength : (nPCBs - 1) * kGasLength / 2;

    Double_t PCBlength;
    Int_t ivol = 0;
    // loop over the number of PCBs in the current slat
    for (auto pcb : PCBs) {

      // if the PCB name starts with a "S", it is a shortened one
      PCBlength = (pcb.front() == 'S') ? kShortPCBLength : kPCBLength;

      // place the corresponding PCB volume in the slat
      slatVol->AddNode(gGeoManager->GetVolume(pcb.data()), ivol + 1,
                       new TGeoTranslation(ivol * kGasLength - 0.5 * (kPCBLength - PCBlength) - center, 0, 0));

      ivol++;
    } // end of the PCBs loop
  }   // end of the slat loop
}

//______________________________________________________________________________
void CreateSupportPanels()
{
  /// Function building the half-chamber support panels (one different per chamber)

  for (Int_t iCh = 5; iCh <= 10; iCh++) {

    // define the support panel volume
    auto support = new TGeoVolumeAssembly(Form("Ch%dSupportPanel", iCh));

    Double_t radius, length, height;
    if (iCh <= 6) { // station 3 half-chambers
      radius = kRadSt3;
      height = kSupportHeightSt3;
      length = (iCh == 5) ? kSupportLengthCh5 : kSupportLengthCh6;
    } else { // station 4 or 5
      radius = kRadSt45;
      length = kSupportLengthSt45;
      height = (iCh <= 8) ? kSupportHeightSt4 : kSupportHeightSt5;
    }

    Double_t width = 0; // increment this value when adding a new layer

    cout << "Support panel for the chamber " << iCh << " : radius = " << radius << ", length = " << length
         << ", height = " << height << endl;
    // create the hole in the nomex volume
    auto nomexHole = new TGeoTube(Form("NomexSupportPanelHoleCh%d", iCh), 0., radius, kNomexSupportWidth / 2.);

    // place this shape on the ALICE x=0 coordinate
    auto holeTrans = new TGeoTranslation(Form("holeCh%dShift", iCh), (-length + kVertSpacerLength) / 2., 0., 0.);
    holeTrans->RegisterYourself();

    // create a box for the nomex volume
    auto nomexBox =
      new TGeoBBox(Form("NomexSupportPanelCh%dBox", iCh), length / 2., height / 2., kNomexSupportWidth / 2.);

    // change the nomex volume shape by extracting the pipe shape
    auto nomexShape =
      new TGeoCompositeShape(Form("NomexSupportPanelCh%dShape", iCh),
                             Form("NomexSupportPanelCh%dBox-NomexSupportPanelHoleCh%d:holeCh%dShift", iCh, iCh, iCh));

    // create the nomex volume and place it in the support panel
    width += kNomexSupportWidth;
    support->AddNode(new TGeoVolume(Form("NomexSupportPanelCh%d", iCh), nomexShape, assertMedium(Medium::Nomex)), iCh,
                     new TGeoTranslation(length / 2., 0., 0.));

    // create the hole in the glue volume
    auto glueHole = new TGeoTube(Form("GlueSupportPanelHoleCh%d", iCh), 0., radius, kGlueSupportWidth / 2.);

    // create a box for the glue volume
    auto glueBox = new TGeoBBox(Form("GlueSupportPanelCh%dBox", iCh), length / 2., height / 2., kGlueSupportWidth / 2.);

    // change the glue volume shape by extracting the pipe shape
    auto glueShape =
      new TGeoCompositeShape(Form("GlueSupportPanelCh%dShape", iCh),
                             Form("GlueSupportPanelCh%dBox-GlueSupportPanelHoleCh%d:holeCh%dShift", iCh, iCh, iCh));

    // create the glue volume
    auto glue = new TGeoVolume(Form("GlueSupportPanelCh%d", iCh), glueShape, assertMedium(Medium::Glue));

    // place it on each side of the nomex volume
    width += kGlueSupportWidth;
    support->AddNode(glue, 1, new TGeoTranslation(length / 2., 0., width / 2.));
    support->AddNode(glue, 2, new TGeoTranslation(length / 2., 0., -width / 2.));

    // create the hole in the carbon volume
    auto carbonHole = new TGeoTube(Form("CarbonSupportPanelHoleCh%d", iCh), 0., radius, kCarbonSupportWidth / 2.);

    // create a box for the carbon volume
    auto carbonBox =
      new TGeoBBox(Form("CarbonSupportPanelCh%dBox", iCh), length / 2., height / 2., kCarbonSupportWidth / 2.);

    // change the carbon volume shape by extracting the pipe shape
    auto carbonShape =
      new TGeoCompositeShape(Form("CarbonSupportPanelCh%dShape", iCh),
                             Form("CarbonSupportPanelCh%dBox-CarbonSupportPanelHoleCh%d:holeCh%dShift", iCh, iCh, iCh));

    // create the carbon volume
    auto carbon = new TGeoVolume(Form("CarbonSupportPanelCh%d", iCh), carbonShape, assertMedium(Medium::Carbon));

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
  for (auto& halfCh : hChs.GetArray()) {
    // check that "halfCh" is an object
    if (!halfCh.IsObject())
      throw runtime_error("Can't create the half-chambers : wrong Value input");

    const Int_t moduleID = halfCh["moduleID"].GetInt();
    const string name = halfCh["name"].GetString();
    // get the chamber number (if the chamber name has a '0' at the 3rd digit, take the number after; otherwise it's the
    // chamber 10)
    const Int_t nCh = (name.find('0') == 2) ? name[3] - '0' /* char -> int conversion */ : 10;

    cout << endl << "Creating half-chamber " << name << endl;
    auto halfChVol = new TGeoVolumeAssembly(name.data());

    // place the support panel corresponding to the chamber number
    auto supRot = new TGeoRotation();
    if (moduleID % 2)
      supRot->RotateY(180.);
    halfChVol->AddNode(gGeoManager->GetVolume(Form("Ch%dSupportPanel", nCh)), moduleID, supRot);

    // place the slat volumes on the different nodes of the half-chamber
    for (auto& slat : halfCh["nodes"].GetArray()) {
      // check that "slat" is an object
      if (!slat.IsObject())
        throw runtime_error("Can't create the slat : wrong Value input");

      const Int_t detID = slat["detID"].GetInt();

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
