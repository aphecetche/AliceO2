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

#include "MCHSimulation/SlatGeometry.h"

using namespace rapidjson;
using namespace std;

const char* gas = "MCH_SlatGas";
const char* carbon = "MCH_Carbon";
const char* nomex = "MCH_Nomex";

namespace o2
{
namespace mch
{

TGeoVolume* NormalPCB(const char* name, Double_t length);

TGeoVolume* RoundedPCB(Double_t curvRad, Double_t yShift);

Double_t SlatCenter(const Int_t nPCBs);

void CreateNormalSlat(const string name);

void CreateRoundedSlat(const string name);

void CreateHalfChambers();

//______________________________________________________________________________
void CreateSlatGeometry()
{
  /// Main function which build and place the slats and the half-chambers volumes
  /// This function must me called by the MCH detector class to build the slat stations geometry.

  // create the normal PCB volumes
  auto shortPCB = NormalPCB("shortPCB", kShortPCBlength);
  auto normalPCB = NormalPCB("normalPCB", kGasDim[0]);

  // create the different slat types
  cout << "Creating " << kSlatTypes.size() << " types of slat" << endl;
  for (auto slatType : kSlatTypes) { // loop over the slat types of the array

    // create a slat volume assembly according to its shape
    (slatType.find('R') < slatType.size()) ? CreateRoundedSlat(slatType) : CreateNormalSlat(slatType);
  }

  // create and place the half-chambers in the top volume
  CreateHalfChambers();
}

//______________________________________________________________________________
Double_t SlatCenter(const Int_t nPCBs)
{
  /// Useful function to compute the center of a slat

  // input : the number of PCBs of the slat
  Double_t x;
  switch (nPCBs % 2) {
    case 0: // even
      x = (nPCBs - 1) * kGasDim[0] / 2;
      break;
    case 1: // odd
      x = (nPCBs / 2) * kGasDim[0];
      break;
  }

  return x;
}

//______________________________________________________________________________
TGeoVolume* NormalPCB(const char* name, Double_t length)
{
  /// Build a normal shape PCB (a simple box of gas volume for the moment)

  // inputs : * the name we want to give to the created volume
  //          * the length (on the x axis) of this PCB

  // get the tracking gas medium
  auto med = gGeoManager->GetMedium(gas);
  if (med == nullptr) {
    throw runtime_error("oups for med");
  }

  return gGeoManager->MakeBox(name, med, length / 2., kGasDim[1] / 2., kGasDim[2] / 2.);
}

//______________________________________________________________________________
TGeoVolume* RoundedPCB(Double_t curvRad, Double_t yShift)
{
  /// Build a rounded shape PCB (a simple gas volume for the moment)

  // inputs : * the radius of curvature of the PCB
  //          * the y position of this PCB w.r.t the LHC beam pipe

  // create a classic shape PCB
  auto PCBshape = new TGeoBBox("PCBshape", kGasDim[0] / 2., kGasDim[1] / 2., kGasDim[2] / 2.);

  // create the beam pipe shape (tube width = slat width)
  auto pipeShape = new TGeoTubeSeg(Form("pipeR%.1fShape", curvRad), 0., curvRad, kSlatWidth / 2., 90., 270.);

  // place this shape on the ALICE x=0; y=0 coordinates
  auto pipeShift =
    new TGeoTranslation(Form("pipeY%.1fShift", yShift), (kGasDim[0] + kVertSpacerLength) / 2., -yShift, 0.);
  pipeShift->RegisterYourself();

  // get the tracking gas medium
  auto med = gGeoManager->GetMedium(gas);
  if (med == nullptr) {
    throw runtime_error("oups for med");
  }

  // return the PCB volume with a new shape
  return new TGeoVolume("roundedPCB",
                        new TGeoCompositeShape(Form("roundedR%.1fY%.1fPCBshape", curvRad, yShift),
                                               Form("PCBshape-pipeR%.1fShape:pipeY%.1fShift", curvRad, yShift)),
                        med);
}

//______________________________________________________________________________
void CreateNormalSlat(const string name)
{
  /// Build a normal shape slat (an assembly of gas volumes for the moment)

  // input : the name of the slat type we want to create

  // get the number of PCBs (= position of the first "0" if there is at least one "0", otherwise it means there are 6
  // PCBs)
  const Int_t nPCBs = (name.find('0') < name.size()) ? name.find('0') : 6;

  cout << "Creating slat type " << name << " which has " << nPCBs << " PCBs" << endl;

  // create the slat volume assembly
  auto slatVol = new TGeoVolumeAssembly(name.data());

  // compute the slat center
  Double_t center = SlatCenter(nPCBs);

  Double_t PCBlength;
  TString PCBshape;
  // loop over the number of PCBs in the current slat
  for (Int_t ivol = 0; ivol < nPCBs; ivol++) {

    PCBlength = kGasDim[0];
    PCBshape = "normalPCB";

    // if the slat name contains a "S", it is a shortened one
    if ((name.back() == 'S') && ivol == nPCBs - 1) {
      PCBlength = kShortPCBlength; // change the last PCB length of the shortened slat
      PCBshape = "shortPCB";       // change the name of the PCB volume
    }

    // place the corresponding PCB volume in the slat
    slatVol->AddNode(gGeoManager->GetVolume(PCBshape), ivol + 1,
                     new TGeoTranslation(ivol * kGasDim[0] - 0.5 * (kGasDim[0] - PCBlength) - center, 0, 0));

  } // end of the PCBs loop
}

//______________________________________________________________________________
void CreateRoundedSlat(const string name)
{
  /// Build a rounded shape slat (an assembly of gas volumes for the moment).
  /// Since the slat will be rotated to match the mapping convention, the PCBs are placed upside down w.r.t the normal
  /// shape slat builder. By doing so, for a shortened slat, we must take care of the PCB shift value.

  // input : the name of the slat type we want to create

  // get the number of PCBs (= 6 for "NR3", 4 for "S(N)R1(2)")
  const Int_t nPCBs = (name.back() == '3') ? 6 : 4;

  cout << "Creating slat type " << name << " which has " << nPCBs << " PCBs" << endl;

  // create the slat volume assembly
  auto slatVol = new TGeoVolumeAssembly(name.data());

  // compute the slat center
  Double_t center = SlatCenter(nPCBs);

  Double_t PCBlength, PCBshift;
  TString PCBshape;
  // loop over the number of normal shape PCBs in the current slat
  for (Int_t ivol = 0; ivol < nPCBs - 1; ivol++) {
    PCBlength = kGasDim[0];
    PCBshift = center;
    PCBshape = "normalPCB";

    // if the slat name contains a "S", it is a shortened one
    if (name.find('S') < name.size() && ivol == 0) {
      PCBlength = kShortPCBlength; // change the first PCB length of the shortened slat
      PCBshift -= 5;               // correct the shift
      PCBshape = "shortPCB";       // change the name of the PCB volume
    }

    // place the corresponding PCB volume in the slat
    slatVol->AddNode(gGeoManager->GetVolume(PCBshape), nPCBs - ivol,
                     new TGeoTranslation(ivol * kGasDim[0] - 0.5 * (kGasDim[0] - PCBlength) - PCBshift, 0, 0));

  } // end of the normal shape PCBs loop

  // LHC beam pipe radius ("NR3" -> it is a slat of a station 4 or 5)
  const Double_t radius = (name.back() == '3') ? kRadSt45 : kRadSt3;

  // y position of the PCB center w.r.t the beam pipe shape
  Double_t ypos;
  switch (name.back()) { // get the last character
    case '1':
      ypos = 0.; // central for "S(N)R1"
      break;
    case '2':
      ypos = kRoundedSlatYposSt3; // "S(N)R2" -> station 3
      break;
    default:
      ypos = kRoundedSlatYposSt45; // "S(N)R3" -> station 4 or 5
      break;
  }

  // place the rounded PCB in the slat volume assembly
  slatVol->AddNode(RoundedPCB(radius + kVertFrameLength, ypos), 1,
                   new TGeoTranslation((nPCBs / 2 - 0.5) * kGasDim[0], 0, 0));
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
    if (!halfCh.IsObject()) {
      cout << "Can't create the half-chambers : wrong Value input" << endl;
      throw;
    }

    cout << endl << "Creating half-chamber " << halfCh["name"].GetString() << endl;
    auto halfChVol = new TGeoVolumeAssembly((const char*)halfCh["name"].GetString());

    // place the slat volumes on the different nodes of the half-chamber
    for (auto& slat : halfCh["nodes"].GetArray()) {
      // check that "slat" is an object
      if (!slat.IsObject()) {
        cout << "Can't create the slat : wrong Value input" << endl;
        throw;
      }

      cout << "Placing the slat " << slat["detID"].GetInt() << " of type " << slat["type"].GetString() << endl;

      // create the slat rotation matrix
      auto slatRot = new TGeoRotation(Form("Slat%drotation", slat["detID"].GetInt()), slat["rotation"][0].GetDouble(),
                                      slat["rotation"][1].GetDouble(), slat["rotation"][2].GetDouble(),
                                      slat["rotation"][3].GetDouble(), slat["rotation"][4].GetDouble(),
                                      slat["rotation"][5].GetDouble());

      // place the slat on the half-chamber volume
      halfChVol->AddNode(
        gGeoManager->GetVolume(slat["type"].GetString()), slat["detID"].GetInt(),
        new TGeoCombiTrans(Form("Slat%dposition", slat["detID"].GetInt()), slat["position"][0].GetDouble(),
                           slat["position"][1].GetDouble(), slat["position"][2].GetDouble(), slatRot));

    } // end of the node loop
    cout << halfCh["nodes"].Size() << " slats placed on the half-chamber " << halfCh["name"].GetString() << endl;

    // place the half-chamber in the top volume
    auto halfChRot = new TGeoRotation(Form("HalfCh%drotation", halfCh["moduleID"].GetInt()),
                                      halfCh["rotation"][0].GetDouble(), halfCh["rotation"][1].GetDouble(),
                                      halfCh["rotation"][2].GetDouble(), halfCh["rotation"][3].GetDouble(),
                                      halfCh["rotation"][4].GetDouble(), halfCh["rotation"][5].GetDouble());

    gGeoManager->GetTopVolume()->AddNode(
      halfChVol, halfCh["moduleID"].GetInt(),
      new TGeoCombiTrans(Form("HalfCh%dposition", halfCh["moduleID"].GetInt()), halfCh["position"][0].GetDouble(),
                         halfCh["position"][1].GetDouble(), halfCh["position"][2].GetDouble(), halfChRot));
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
      "position":[0.0000, -0.1663, -967.4988],
      "rotation":[90.0000, 0.0000, 90.7940, 90.0000, 0.7940, 90.0000],
      "nodes":[
        {
          "detID":500,
          "type":"122000SR1",
          "position":[81.2500, 0.0000, 11.7500],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":501,
          "type":"112200SR2",
          "position":[81.2500, 37.8000, 3.7500],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":502,
          "type":"122200S",
          "position":[81.2500, 75.5000, 11.7500],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":503,
          "type":"222000N",
          "position":[61.2500, 112.8000, 3.7500],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":504,
          "type":"220000N",
          "position":[41.2500, 146.5000, 11.7500],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":514,
          "type":"220000N",
          "position":[41.2500, -146.5000, 11.7500],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":515,
          "type":"222000N",
          "position":[61.2500, -112.8000, 3.7500],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":516,
          "type":"122200S",
          "position":[81.2500, -75.5000, 11.7500],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":517,
          "type":"112200SR2",
          "position":[81.2500, -37.8000, 3.7500],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        }
      ]
    },

    {
      "name":"SC05O",
      "moduleID":5,
      "position":[0.0000, 0.1663, -967.5012],
      "rotation":[90.0000, 0.0000, 90.7940, 90.0000, 0.7940, 90.0000],
      "nodes":[
        {
          "detID":505,
          "type":"220000N",
          "position":[-41.2500, 146.5000, -11.7500],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        },
        {
          "detID":506,
          "type":"222000N",
          "position":[-61.2500, 112.8000, -3.7500],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":507,
          "type":"122200S",
          "position":[-81.2500, 75.5000, -11.7500],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        },
        {
          "detID":508,
          "type":"112200SR2",
          "position":[-81.2500, 37.8000, -3.7500],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":509,
          "type":"122000SR1",
          "position":[-81.2500, 0.0000, -11.7500],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":510,
          "type":"112200SR2",
          "position":[-81.2500, -37.8000, -3.7500],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":511,
          "type":"122200S",
          "position":[-81.2500, -75.5000, -11.7500],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        },
        {
          "detID":512,
          "type":"222000N",
          "position":[-61.2500, -112.8000, -3.7500],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":513,
          "type":"220000N",
          "position":[-41.2500, -146.5000, -11.7500],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        }
      ]
    },

    {
      "name":"SC06I",
      "moduleID":6,
      "position":[0.0000, -0.1663, -998.4988],
      "rotation":[90.0000, 0.0000, 90.7940, 90.0000, 0.7940, 90.0000],
      "nodes":[
        {
          "detID":600,
          "type":"122000NR1",
          "position":[81.2500, 0.0000, 11.7500],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":601,
          "type":"112200NR2",
          "position":[81.2500, 37.8000, 3.7500],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":602,
          "type":"122200N",
          "position":[81.2500, 75.5000, 11.7500],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":603,
          "type":"222000N",
          "position":[61.2500, 112.8000, 3.7500],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":604,
          "type":"220000N",
          "position":[41.2500, 146.5000, 11.7500],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":614,
          "type":"220000N",
          "position":[41.2500, -146.5000, 11.7500],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":615,
          "type":"222000N",
          "position":[61.2500, -112.8000, 3.7500],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":616,
          "type":"122200N",
          "position":[81.2500, -75.5000, 11.7500],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":617,
          "type":"112200NR2",
          "position":[81.2500, -37.8000, 3.7500],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        }
      ]
    },

    {
      "name":"SC06O",
      "moduleID":7,
      "position":[0.0000, 0.1663, -998.5012],
      "rotation":[90.0000, 0.0000, 90.7940, 90.0000, 0.7940, 90.0000],
      "nodes":[
        {
          "detID":605,
          "type":"220000N",
          "position":[-41.2500, 146.5000, -11.7500],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        },
        {
          "detID":606,
          "type":"222000N",
          "position":[-61.2500, 112.8000, -3.7500],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":607,
          "type":"122200N",
          "position":[-81.2500, 75.5000, -11.7500],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        },
        {
          "detID":608,
          "type":"112200NR2",
          "position":[-81.2500, 37.8000, -3.7500],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":609,
          "type":"122000NR1",
          "position":[-81.2500, 0.0000, -11.7500],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":610,
          "type":"112200NR2",
          "position":[-81.2500, -37.8000, -3.7500],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":611,
          "type":"122200N",
          "position":[-81.2500, -75.5000, -11.7500],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        },
        {
          "detID":612,
          "type":"222000N",
          "position":[-61.2500, -112.8000, -3.7500],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":613,
          "type":"220000N",
          "position":[-41.2500, -146.5000, -11.7500],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        }
      ]
    },

    {
      "name":"SC07I",
      "moduleID":8,
      "position":[0.0000, -0.1663, -1276.4988],
      "rotation":[90.0000, 0.0000, 90.7940, 90.0000, 0.7940, 90.0000],
      "nodes":[
        {
          "detID":700,
          "type":"122330N",
          "position":[140.0000, 0.0000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":701,
          "type":"112233NR3",
          "position":[121.2500, 38.2000, 3.5000],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":702,
          "type":"112230N",
          "position":[101.2500, 72.6000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":703,
          "type":"222330N",
          "position":[101.2500, 109.2000, 3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":704,
          "type":"223300N",
          "position":[81.2500, 138.5000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":705,
          "type":"333000N",
          "position":[61.2500, 175.5000, 3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":706,
          "type":"330000N",
          "position":[41.2500, 204.5000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":720,
          "type":"330000N",
          "position":[41.2500, -204.5000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":721,
          "type":"333000N",
          "position":[61.2500, -175.5000, 3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":722,
          "type":"223300N",
          "position":[81.2500, -138.5000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":723,
          "type":"222330N",
          "position":[101.2500, -109.2000, 3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":724,
          "type":"112230N",
          "position":[101.2500, -72.6000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":725,
          "type":"112233NR3",
          "position":[121.2500, -38.2000, 3.5000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        }
      ]
    },

    {
      "name":"SC07O",
      "moduleID":9,
      "position":[0.0000, -0.1663, -1276.5012],
      "rotation":[90.0000, 0.0000, 90.7940, 90.0000, 0.7940, 90.0000],
      "nodes":[
        {
          "detID":707,
          "type":"330000N",
          "position":[-41.2500, 204.5000, -12.0000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        },
        {
          "detID":708,
          "type":"333000N",
          "position":[-61.2500, 175.5000, -3.5000],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":709,
          "type":"223300N",
          "position":[-81.2500, 138.5000, -12.0000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        },
        {
          "detID":710,
          "type":"222330N",
          "position":[-101.2500, 109.2000, -3.5000],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":711,
          "type":"112230N",
          "position":[-101.2500, 72.6000, -12.0000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        },
        {
          "detID":712,
          "type":"112233NR3",
          "position":[-121.2500, 38.2000, -3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":713,
          "type":"122330N",
          "position":[-140.0000, 0.0000, -12.0000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        },
        {
          "detID":714,
          "type":"112233NR3",
          "position":[-121.2500, -38.2000, -3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":715,
          "type":"112230N",
          "position":[-101.2500, -72.6000, -12.0000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        },
        {
          "detID":716,
          "type":"222330N",
          "position":[-101.2500, -109.2000, -3.5000],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":717,
          "type":"223300N",
          "position":[-81.2500, -138.5000, -12.0000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        },
        {
          "detID":718,
          "type":"333000N",
          "position":[-61.2500, -175.5000, -3.5000],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":719,
          "type":"330000N",
          "position":[-41.2500, -204.5000, -12.0000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        }
      ]
    },

    {
      "name":"SC08I",
      "moduleID":10,
      "position":[0.0000, -0.1663, -1307.4988],
      "rotation":[90.0000, 0.0000, 90.7940, 90.0000, 0.7940, 90.0000],
      "nodes":[
        {
          "detID":800,
          "type":"122330N",
          "position":[140.0000, 0.0000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":801,
          "type":"112233NR3",
          "position":[121.2500, 38.2000, 3.5000],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":802,
          "type":"112230N",
          "position":[101.2500, 76.0500, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":803,
          "type":"222330N",
          "position":[101.2500, 113.6000, 3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":804,
          "type":"223300N",
          "position":[81.2500, 143.0000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":805,
          "type":"333000N",
          "position":[61.2500, 180.0000, 3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":806,
          "type":"330000N",
          "position":[41.2500, 208.6000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":820,
          "type":"330000N",
          "position":[41.2500, -208.6000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":821,
          "type":"333000N",
          "position":[61.2500, -180.000, 3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":822,
          "type":"223300N",
          "position":[81.2500, -143.0000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":823,
          "type":"222330N",
          "position":[101.2500, -113.6000, 3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":824,
          "type":"112230N",
          "position":[101.2500, -76.0500, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":825,
          "type":"112233NR3",
          "position":[121.2500, -38.2000, 3.5000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        }
      ]
    },

    {
      "name":"SC08O",
      "moduleID":11,
      "position":[0.0000, -0.1663, -1307.5012],
      "rotation":[90.0000, 0.0000, 90.7940, 90.0000, 0.7940, 90.0000],
      "nodes":[
        {
          "detID":807,
          "type":"330000N",
          "position":[-41.2500, 208.6000, -12.0000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        },
        {
          "detID":808,
          "type":"333000N",
          "position":[-61.2500, 180.0000, -3.5000],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":809,
          "type":"223300N",
          "position":[-81.2500, 143.0000, -12.0000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        },
        {
          "detID":810,
          "type":"222330N",
          "position":[-101.2500, 113.6000, -3.5000],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":811,
          "type":"112230N",
          "position":[-101.2500, 76.0500, -12.0000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        },
        {
          "detID":812,
          "type":"112233NR3",
          "position":[-121.2500, 38.2000, -3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":813,
          "type":"122330N",
          "position":[-140.0000, 0.0000, -12.0000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        },
        {
          "detID":814,
          "type":"112233NR3",
          "position":[-121.2500, -38.2000, -3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":815,
          "type":"112230N",
          "position":[-101.2500, -76.0500, -12.0000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        },
        {
          "detID":816,
          "type":"222330N",
          "position":[-101.2500, -113.6000, -3.5000],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":817,
          "type":"223300N",
          "position":[-81.2500, -143.0000, -12.0000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        },
        {
          "detID":818,
          "type":"333000N",
          "position":[-61.2500, -180.0000, -3.5000],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":819,
          "type":"330000N",
          "position":[-41.2500, -208.6000, -12.0000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        }
      ]
    },

    {
      "name":"SC09I",
      "moduleID":12,
      "position":[0.0000, -0.1663, -1406.4988],
      "rotation":[90.0000, 0.0000, 90.7940, 90.0000, 0.7940, 90.0000],
      "nodes":[
        {
          "detID":900,
          "type":"122330N",
          "position":[140.0000, 0.0000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":901,
          "type":"112233NR3",
          "position":[121.2500, 38.2000, 3.5000],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":902,
          "type":"112233N",
          "position":[121.2500, 76.1000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":903,
          "type":"222333N",
          "position":[121.2500, 113.7000, 3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":904,
          "type":"223330N",
          "position":[101.2500, 151.0000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":905,
          "type":"333300N",
          "position":[81.2500, 188.0500, 3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":906,
          "type":"333000N",
          "position":[61.2500, 224.8000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":920,
          "type":"333000N",
          "position":[61.2500, -224.8000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":921,
          "type":"333300N",
          "position":[81.2500, -188.0500, 3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":922,
          "type":"223330N",
          "position":[101.2500, -151.0000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":923,
          "type":"222333N",
          "position":[121.2500, -113.7000, 3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":924,
          "type":"112233N",
          "position":[121.2500, -76.1000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":925,
          "type":"112233NR3",
          "position":[121.2500, -38.2000, 3.5000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        }
      ]
    },

    {
      "name":"SC09O",
      "moduleID":13,
      "position":[0.0000, -0.1663, -1406.5012],
      "rotation":[90.0000, 0.0000, 90.7940, 90.0000, 0.7940, 90.0000],
      "nodes":[
        {
          "detID":907,
          "type":"333000N",
          "position":[-61.2500, 224.8000, -12.0000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        },
        {
          "detID":908,
          "type":"333300N",
          "position":[-81.2500, 188.0500, -3.5000],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":909,
          "type":"223330N",
          "position":[-101.2500, 151.0000, -12.0000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        },
        {
          "detID":910,
          "type":"222333N",
          "position":[-121.2500, 113.7000, -3.5000],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":911,
          "type":"112233N",
          "position":[-121.2500, 76.1000, -12.0000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        },
        {
          "detID":912,
          "type":"112233NR3",
          "position":[-121.2500, 38.2000, -3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":913,
          "type":"122330N",
          "position":[-140.0000, 0.0000, -12.0000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        },
        {
          "detID":914,
          "type":"112233NR3",
          "position":[-121.2500, -38.2000, -3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":915,
          "type":"112233N",
          "position":[-121.2500, -76.1000, -12.0000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        },
        {
          "detID":916,
          "type":"222333N",
          "position":[-121.2500, -113.7000, -3.5000],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":917,
          "type":"223330N",
          "position":[-101.2500, -151.0000, -12.0000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        },
        {
          "detID":918,
          "type":"333300N",
          "position":[-81.2500, -188.0500, -3.5000],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":919,
          "type":"333000N",
          "position":[-61.2500, -224.8000, -12.0000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        }
      ]
    },

    {
      "name":"SC10I",
      "moduleID":14,
      "position":[0.0000, -0.1663, -1437.4988],
      "rotation":[90.0000, 0.0000, 90.7940, 90.0000, 0.7940, 90.0000],
      "nodes":[
        {
          "detID":1000,
          "type":"122330N",
          "position":[140.0000, 0.0000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":1001,
          "type":"112233NR3",
          "position":[121.2500, 38.2000, 3.5000],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":1002,
          "type":"112233N",
          "position":[121.2500, 76.1000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":1003,
          "type":"222333N",
          "position":[121.2500, 113.7000, 3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":1004,
          "type":"223330N",
          "position":[101.2500, 151.0000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":1005,
          "type":"333300N",
          "position":[81.2500, 188.0500, 3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":1006,
          "type":"333000N",
          "position":[61.2500, 224.8000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":1020,
          "type":"333000N",
          "position":[61.2500, -224.8000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":1021,
          "type":"333300N",
          "position":[81.2500, -188.0500, 3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":1022,
          "type":"223330N",
          "position":[101.2500, -151.0000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":1023,
          "type":"222333N",
          "position":[121.2500, -113.7000, 3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":1024,
          "type":"112233N",
          "position":[121.2500, -76.1000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":1025,
          "type":"112233NR3",
          "position":[121.2500, -38.2000, 3.5000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        }
      ]
    },

    {
      "name":"SC10O",
      "moduleID":15,
      "position":[0.0000, -0.1663, -1437.5012],
      "rotation":[90.0000, 0.0000, 90.7940, 90.0000, 0.7940, 90.0000],
      "nodes":[
        {
          "detID":1007,
          "type":"333000N",
          "position":[-61.2500, 224.8000, -12.0000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        },
        {
          "detID":1008,
          "type":"333300N",
          "position":[-81.2500, 188.0500, -3.5000],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":1009,
          "type":"223330N",
          "position":[-101.2500, 151.0000, -12.0000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        },
        {
          "detID":1010,
          "type":"222333N",
          "position":[-121.2500, 113.7000, -3.5000],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":1011,
          "type":"112233N",
          "position":[-121.2500, 76.1000, -12.0000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        },
        {
          "detID":1012,
          "type":"112233NR3",
          "position":[-121.2500, 38.2000, -3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":1013,
          "type":"122330N",
          "position":[-140.0000, 0.0000, -12.0000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        },
        {
          "detID":1014,
          "type":"112233NR3",
          "position":[-121.2500, -38.2000, -3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":1015,
          "type":"112233N",
          "position":[-121.2500, -76.1000, -12.0000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        },
        {
          "detID":1016,
          "type":"222333N",
          "position":[-121.2500, -113.7000, -3.5000],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":1017,
          "type":"223330N",
          "position":[-101.2500, -151.0000, -12.0000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        },
        {
          "detID":1018,
          "type":"333300N",
          "position":[-81.2500, -188.0500, -3.5000],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":1019,
          "type":"333000N",
          "position":[-61.2500, -224.8000, -12.0000],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        }
      ]
    }
  ]
}
)";
} // namespace mch
} // namespace o2
