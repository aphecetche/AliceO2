#include <iostream>
#include "TCanvas.h"
#include "TGeoCompositeShape.h"
#include "TGeoManager.h"
#include "TGeoMedium.h"
#include "TGeoShape.h"
#include "TGeoTube.h"
#include "TGeoVolume.h"
#include "TROOT.h"
#include "TStopwatch.h"
#include "TSystem.h"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "SlatGeometry.h"
#include "SlatMaterials.h"

using namespace rapidjson;
using std::cout;
using std::endl;

namespace o2
{
namespace mch
{

TGeoShape* NormalPCBshape(const char* name, Double_t length);

TGeoVolume* NormalPCB(const char* name, Double_t length, Color_t color);

TGeoVolume* CentralPCB();

TGeoVolume* DownRoundedPCB(Double_t curvRad, Double_t yShift);

TGeoVolume* UpRoundedPCB(Double_t curvRad, Double_t yShift);

TGeoVolumeAssembly* NormalSlat(Int_t detID, Int_t nPCBs);

TGeoVolumeAssembly* CentralSlat(Int_t detID, Int_t nPCBs);

TGeoVolumeAssembly* RoundedSlat(Int_t detID, Int_t nPCBs, Double_t yPipe, Double_t radius);

void CreateChamber(Value& chamber);

void CreateStation(Value& station);

//  Constants
const Double_t kCentPCBlength = 19.25; // central PCB length
const Double_t kVertFramelength = 2.;  // space between the central PCB edge and the beam pipe
const Double_t kVertSpacerLength = 2.5;
const Double_t kGasDim[3] = { 40., 40., 0.5 };

// Inner radii
const Double_t kRadSt3 = 29.5;
const Double_t kRadSt45 = 37.5;

// Slat chambers common parameters
const Double_t kRotSlatCh[6] = { 90.0000, 0.0000, 90.7940, 90.0000, 0.7940, 90.0000 };
const Double_t kXPosSlatCh = 0.;
const Double_t kYPosSlatCh = 0.1663; // be careful with the sign !

void createSlatGeometry()
{
  createSlatMaterials();

  materialManager().printMaterials();
  materialManager().printMedia();

  StringStream is(jsonSlatDescription.c_str());

  Document doc;
  doc.ParseStream(is);

  Value& chambers = doc["chambers"];
  assert(chambers.IsArray());

  // loop over the elements of the chamber array
  for (SizeType i = 0; i < chambers.Size(); i++) {

    Value& ch = chambers[i];
    assert(ch.IsObject());

    // create the chamber : build and position a half-chamber (volume assembly)
    // and its copy
    switch (ch["stnumber"].GetInt()) {
      case 3: // St3 chambers are treated separately
        CreateChamber(ch);
        break;
      default:
        // CreateStation(ch);
        break;
    }

  } // end of the chamber loop
}

TGeoShape* NormalPCBshape(const char* name, Double_t length)
{
  // Useful intermediate method, will be an inline one in O2
  return new TGeoBBox(name, length / 2., kGasDim[1] / 2., kGasDim[2] / 2.);
}

TGeoVolume* NormalPCB(const char* name, Double_t length, Color_t color)
{

  auto vol = gGeoManager->MakeBox(name, assertMedium(Medium::Vacuum), length / 2., kGasDim[1] / 2., kGasDim[2] / 2.);
  vol->SetVisibility(kTRUE);
  vol->SetTransparency(0);
  vol->SetLineColor(color);
  vol->SetFillColor(color);

  return vol;
}

TGeoVolume* CentralPCB()
{

  // create a classic shape PCB
  auto PCBshape = NormalPCBshape("PCBshape", kGasDim[0]);

  // create the beam pipe shape (tube width = gas volume width)
  auto pipeShape = new TGeoTubeSeg("pipeShape", 0., kRadSt3 + kVertFramelength, kGasDim[2] / 2., -90., 90.);

  // place this shape on the ALICE x=0 coordinate
  auto pipeShift = new TGeoTranslation("pipeShift", -kGasDim[0] / 2., 0., 0.);
  pipeShift->RegisterYourself();

  auto vol = new TGeoVolume("centralPCB", new TGeoCompositeShape("cPCBshape", "PCBshape-pipeShape:pipeShift"),
                            assertMedium(Medium::ArCo2));
  vol->SetVisibility(kTRUE);
  vol->SetTransparency(0);
  vol->SetLineColor(kGreen + 1);
  vol->SetFillColor(kGreen + 1);

  return vol;
}

TGeoVolume* DownRoundedPCB(Double_t curvRad, Double_t yShift)
{

  // create a classic shape PCB
  auto PCBshape = NormalPCBshape("PCBshape", kGasDim[0]);

  // create the beam pipe shape (tube width = gas volume width)
  auto pipeShape = new TGeoTubeSeg(Form("downpipe%.1fShape", curvRad), 0., curvRad, kGasDim[2] / 2., 0., 90.);

  // place this shape on the ALICE x=0; y=0 coordinate
  auto pipeShift =
    new TGeoTranslation(Form("downpipe%.1fShift", curvRad), -(kGasDim[0] + kVertSpacerLength) / 2., -yShift, 0.);
  pipeShift->RegisterYourself();

  auto vol =
    new TGeoVolume("downroundedPCB",
                   new TGeoCompositeShape(Form("downrounded%.1fPCBshape", curvRad),
                                          Form("PCBshape-downpipe%.1fShape:downpipe%.1fShift", curvRad, curvRad)),
                   assertMedium(Medium::ArCo2));
  vol->SetVisibility(kTRUE);
  vol->SetTransparency(0);
  vol->SetLineColor(kOrange - 2);
  vol->SetFillColor(kOrange - 2);

  return vol;
}

TGeoVolume* UpRoundedPCB(Double_t curvRad, Double_t yShift)
{

  // create a classic shape PCB
  auto PCBshape = NormalPCBshape("PCBshape", kGasDim[0]);

  // create the beam pipe shape (tube width = gas volume width)
  auto pipeShape = new TGeoTubeSeg(Form("uppipe%.1fShape", curvRad), 0., curvRad, kGasDim[2] / 2., 270., 0.);

  // place this shape on the ALICE x=0; y=0 coordinate
  auto pipeShift =
    new TGeoTranslation(Form("uppipe%.1fShift", curvRad), -(kGasDim[0] + kVertSpacerLength) / 2., -yShift, 0.);
  pipeShift->RegisterYourself();

  auto vol = new TGeoVolume("uproundedPCB",
                            new TGeoCompositeShape(Form("uprounded%.1fPCBShape", curvRad),
                                                   Form("PCBshape-uppipe%.1fShape:uppipe%.1fShift", curvRad, curvRad)),
                            assertMedium(Medium::ArCo2));
  vol->SetVisibility(kTRUE);
  vol->SetTransparency(0);
  vol->SetLineColor(kOrange - 2);
  vol->SetFillColor(kOrange - 2);

  return vol;
}

TGeoVolumeAssembly* NormalSlat(Int_t detID, Int_t nPCBs)
{

  // create the slat volume assembly
  auto slatVol = new TGeoVolumeAssembly(Form("Slat%d", detID));

  // shift the origin of the local slat coordinates to the center of the slat
  Double_t shift;
  switch (nPCBs % 2) {
    case 0: // even
      shift = (nPCBs - 1) * kGasDim[0] / 2;
      break;
    case 1: // odd
      shift = (nPCBs / 2) * kGasDim[0];
      break;
  }

  // to have a nice view of the slats when drawing
  Color_t color;
  if (detID % 2)
    color = kBlue - 3;
  else
    color = kAzure + 7;

  Double_t PCBlength;
  // loop over the number of PCBs in the current slat
  for (Int_t ivol = 0; ivol < nPCBs; ivol++) {

    PCBlength = kGasDim[0];

    if ((detID == 502 || detID == 516) && ivol == nPCBs - 1)
      PCBlength -= 5; // take off 5cm for the last PCB

    // create the PCB volume
    auto PCBvol = NormalPCB(Form("normalPCB%d", ivol), PCBlength, color);

    // place the PCB in the slat volume assembly and shift the origin to the
    // center of the slat
    slatVol->AddNode(PCBvol, detID,
                     new TGeoTranslation(ivol * kGasDim[0] - 0.5 * (kGasDim[0] - PCBlength) - shift, 0, 0));

  } // end of the PCBs loop

  return slatVol;
}

TGeoVolumeAssembly* CentralSlat(Int_t detID, Int_t nPCBs)
{

  // create the slat volume assembly
  auto slatVol = new TGeoVolumeAssembly(Form("Slat%d", detID));

  // compute the slat center
  Double_t shift;
  switch (nPCBs % 2) {
    case 0: // even
      shift = (nPCBs - 1) * kGasDim[0] / 2;
      break;
    case 1: // odd
      shift = (nPCBs / 2) * kGasDim[0];
      break;
  }

  // to have a nice view of the slats when drawing
  Color_t color;
  if (detID % 2)
    color = kBlue - 3;
  else
    color = kAzure + 7;

  // First, create the central PCB volume
  auto cPCBvol = CentralPCB();

  // place the PCB in the slat volume assembly
  slatVol->AddNode(cPCBvol, detID, new TGeoTranslation(-(nPCBs / 2 - 0.5) * kGasDim[0], 0, 0));

  Double_t PCBlength;
  // loop over the number of PCBs in the current slat
  // /!\ in this loop, we only build the normal shape PCBs (that's why we start
  // at ivol=1)
  for (Int_t ivol = 1; ivol < nPCBs; ivol++) {
    PCBlength = kGasDim[0];

    if (detID == 500 && ivol == nPCBs - 1)
      PCBlength -= 5; // take off 5cm for the last PCB

    // create the PCB volume
    auto PCBvol = NormalPCB(Form("normalPCB%d", ivol), PCBlength, color);

    // place the PCB in the slat volume assembly and shift the origin to the
    // center of the slat
    slatVol->AddNode(PCBvol, detID,
                     new TGeoTranslation(ivol * kGasDim[0] - 0.5 * (kGasDim[0] - PCBlength) - shift, 0, 0));

  } // end of the normal shape PCBs loop

  return slatVol;
}

TGeoVolumeAssembly* RoundedSlat(Int_t detID, Int_t nPCBs, Double_t radius, Double_t yPipe)
{

  // create the slat volume assembly
  auto slatVol = new TGeoVolumeAssembly(Form("Slat%d", detID));

  // compute the slat center
  Double_t shift;
  switch (nPCBs % 2) {
    case 0: // even
      shift = (nPCBs - 1) * kGasDim[0] / 2;
      break;
    case 1: // odd
      shift = (nPCBs / 2) * kGasDim[0];
      break;
  }

  // to have a nice view of the slats when drawing
  Color_t color;
  if (detID % 2)
    color = kBlue - 3;
  else
    color = kAzure + 7;

  // First, create the rounded PCB volume
  auto rPCBvol = new TGeoVolume();
  switch (TMath::Sign(1, yPipe)) {
    case 1: // y>0
      rPCBvol = DownRoundedPCB(radius + kVertFramelength, yPipe);
      break;
    case -1: // y<0
      rPCBvol = UpRoundedPCB(radius + kVertFramelength, yPipe);
      break;
  }

  // place the PCB in the slat volume assembly
  slatVol->AddNode(rPCBvol, detID, new TGeoTranslation(-(nPCBs / 2 - 0.5) * kGasDim[0], 0, 0));

  Double_t PCBlength;
  // loop over the number of PCBs in the current slat
  // /!\ in this loop, we only build the normal shape PCBs (that's why we start
  // at ivol=1)
  for (Int_t ivol = 1; ivol < nPCBs; ivol++) {
    PCBlength = kGasDim[0];

    if (detID == 501 && ivol == nPCBs - 1)
      PCBlength -= 5; // take off 5cm for the last PCB

    // create the PCB volume
    auto PCBvol = NormalPCB(Form("normalPCB%d", ivol), PCBlength, color);

    // place the PCB in the slat volume assembly and shift the origin to the
    // center of the slat
    slatVol->AddNode(PCBvol, detID,
                     new TGeoTranslation(ivol * kGasDim[0] - 0.5 * (kGasDim[0] - PCBlength) - shift, 0, 0));

  } // end of the normal shape PCBs loop

  return slatVol;
}

void CreateChamber(Value& ch)
{
  // check that "ch" is an object
  if (!ch.IsObject()) {
    cout << "Can't create the chamber : wrong Value input" << endl;
    return;
  }

  if (!ch["name"].IsArray()) {
    cout << "Not what I expected\n";
    throw;
  }

  cout << "-- CreateChamber --\n";

  for (auto& x : ch["name"].GetArray()) {
    cout << x.GetString() << " - ";
  }
  // create a half-chamber volume assembly (the inner one)
  auto InVol = new TGeoVolumeAssembly((const char*)ch["name"][0].GetString());

  // create the outer one where the slat copies will be placed
  auto OutVol = new TGeoVolumeAssembly((const char*)ch["name"][1].GetString());

  // create the half-chamber rotation matrix
  auto hChRot = new TGeoRotation("half-chamber rotation", kRotSlatCh[0], kRotSlatCh[1], kRotSlatCh[2], kRotSlatCh[3],
                                 kRotSlatCh[4], kRotSlatCh[5]);
  hChRot->RegisterYourself();

  // get the top volume and place the half-chambers in it

  gGeoManager->GetTopVolume()->AddNode(
    InVol, ch["moduleID"][0].GetInt(),
    new TGeoCombiTrans(kXPosSlatCh, -kYPosSlatCh, ch["zpos"][0].GetDouble(), hChRot));

  gGeoManager->GetTopVolume()->AddNode(OutVol, ch["moduleID"][1].GetInt(),
                                       new TGeoCombiTrans(kXPosSlatCh, kYPosSlatCh, ch["zpos"][1].GetDouble(), hChRot));

  // create a "slats" array containing the slat objects of the current chamber
  Value& slats = ch["slats"];
  assert(slats.IsArray());

  cout << slats.GetArray().Size() << " slats\n";

  TString shape;
  Double_t pos[3], rot[6];
  Int_t detID, nPCBs;

  // Slat rotation
  auto rotcopy = new TGeoRotation();
  rotcopy->RotateY(180);

  for (SizeType iSlat = 0; iSlat < slats.Size(); iSlat++) { // loop over the slats of the "slats" array

    // create a "slat" object containing the needed informations
    Value& slat = slats[iSlat];
    assert(slat.IsObject());

    shape = slat["shape"].GetString();
    detID = slat["detID"][0].GetInt();
    nPCBs = slat["nPCBs"].GetInt();

    // fill the slat position array
    for (Int_t i = 0; i < 3; i++) {
      pos[i] = slat["position"][i].GetDouble();
    }

    // fill the slat rotation array
    for (Int_t i = 0; i < 6; i++) {
      rot[i] = slat["rotation"][i].GetDouble();
    }

    TGeoVolume* slatVol{ nullptr }; // -LA- new TGeoVolume();
    // create a slat volume assembly according to its shape
    if (shape == "central")
      slatVol = CentralSlat(detID, nPCBs);

    else if (shape == "rounded")
      slatVol = RoundedSlat(detID, nPCBs, kRadSt3, pos[1]);

    else
      slatVol = NormalSlat(detID, nPCBs);

    // place the slat volume on the half-chamber volume
    InVol->AddNode(slatVol, slat["detID"][0].GetInt(), new TGeoTranslation(pos[0], pos[1], pos[2]));

    OutVol->AddNode(slatVol, slat["detID"][1].GetInt(), new TGeoCombiTrans(-pos[0], pos[1], -pos[2], rotcopy));

  } // end of the slat loop
}

void CreateStation(Value& ch)
{
  // check that "ch" is an object
  if (!ch.IsObject()) {
    cout << "Can't create the station : wrong Value input" << endl;
    return;
  }

  // create a half-chamber volume assembly (the inner one)
  auto InVol = new TGeoVolumeAssembly((const char*)ch["name"][0].GetString());

  // create the outer one where the slat copies will be placed
  auto OutVol = new TGeoVolumeAssembly((const char*)ch["name"][1].GetString());

  // create a second half-chamber volume assembly (for the other chamber)
  auto InVolSec = new TGeoVolumeAssembly((const char*)ch["name"][2].GetString());

  // create the outer one where the slat copies will be placed
  auto OutVolSec = new TGeoVolumeAssembly((const char*)ch["name"][3].GetString());

  // create the half-chamber rotation matrix
  auto hChRot = new TGeoRotation("half-chamber rotation", kRotSlatCh[0], kRotSlatCh[1], kRotSlatCh[2], kRotSlatCh[3],
                                 kRotSlatCh[4], kRotSlatCh[5]);
  hChRot->RegisterYourself();

  // get the top volume and place the half-chambers in it
  gGeoManager->GetTopVolume()->AddNode(
    InVol, ch["moduleID"][0].GetInt(),
    new TGeoCombiTrans(kXPosSlatCh, -kYPosSlatCh, ch["zpos"][0].GetDouble(), hChRot));

  gGeoManager->GetTopVolume()->AddNode(OutVol, ch["moduleID"][1].GetInt(),
                                       new TGeoCombiTrans(kXPosSlatCh, kYPosSlatCh, ch["zpos"][1].GetDouble(), hChRot));

  gGeoManager->GetTopVolume()->AddNode(
    InVolSec, ch["moduleID"][2].GetInt(),
    new TGeoCombiTrans(kXPosSlatCh, -kYPosSlatCh, ch["zpos"][2].GetDouble(), hChRot));

  gGeoManager->GetTopVolume()->AddNode(OutVolSec, ch["moduleID"][3].GetInt(),

                                       new TGeoCombiTrans(kXPosSlatCh, kYPosSlatCh, ch["zpos"][3].GetDouble(), hChRot));

  // create a "slats" array containing the slat objects of the current station
  Value& slats = ch["slats"];
  assert(slats.IsArray());

  TString shape;
  Int_t detID, nPCBs;
  Double_t pos[3], rot[6];

  // Slat rotation
  auto rotcopy = new TGeoRotation();
  rotcopy->RotateY(180);

  for (SizeType iSlat = 0; iSlat < slats.Size(); iSlat++) { // loop over the slats of the "slats" array

    // create a "slat" object containing the needed informations
    Value& slat = slats[iSlat];
    assert(slat.IsObject());

    shape = slat["shape"].GetString();
    detID = slat["detID"][0].GetInt();
    nPCBs = slat["nPCBs"].GetInt();

    // fill the slat position array
    for (Int_t i = 0; i < 3; i++) {
      pos[i] = slat["position"][i].GetDouble();
    }

    // fill the slat rotation array
    for (Int_t i = 0; i < 6; i++) {
      rot[i] = slat["rotation"][i].GetDouble();
    }

    auto slatVol = new TGeoVolume();
    // create a slat volume assembly according to its shape
    if (shape == "rounded")
      slatVol = RoundedSlat(detID, nPCBs, kRadSt3, pos[1]);

    else
      slatVol = NormalSlat(detID, nPCBs);

    // copy the slat volume
    auto slatCopy1 = slatVol->CloneVolume();
    slatCopy1->SetName((Form("Slat%d", slat["detID"][1].GetInt())));

    auto slatCopy2 = slatVol->CloneVolume();
    slatCopy2->SetName((Form("Slat%d", slat["detID"][2].GetInt())));

    auto slatCopy3 = slatVol->CloneVolume();
    slatCopy3->SetName((Form("Slat%d", slat["detID"][3].GetInt())));
    // place the slat volume on the half-chamber volume
    // InVol->AddNode(slatVol,1,new TGeoCombiTrans(pos[0],pos[1],pos[2], new
    // TGeoRotation("rotslat",rot[0],rot[1],rot[2],rot[3],rot[4],rot[5])));
    InVol->AddNode(slatVol, 1 + iSlat * 4, new TGeoTranslation(pos[0], pos[1], pos[2]));

    // OutVol->AddNode(slatVol,1,new TGeoCombiTrans(pos[0],pos[1],pos[2], new
    // TGeoRotation("rotslatcopy",rot[0],rot[1]+180,rot[2],rot[3],rot[4]+180,rot[5])));

    // place the slat copies
    OutVol->AddNode(slatCopy1, 1 + iSlat * 4 + 1, new TGeoCombiTrans(-pos[0], pos[1], -pos[2], rotcopy));

    InVolSec->AddNode(slatCopy2, 1 + iSlat * 4 + 2, new TGeoTranslation(pos[0], pos[1], pos[2]));

    OutVolSec->AddNode(slatCopy3, 1 + iSlat * 4 + 3, new TGeoCombiTrans(-pos[0], pos[1], -pos[2], rotcopy));
  } // end of the slat loop
}

const std::string jsonSlatDescription =
  R"(
{
  "chambers": [
    {
      "stnumber":3,
      "name":["SC05I","SC05O"],
      "moduleID":[4,5],
      "type":"slat",
      "zpos":[-967.4988,-967.5012],
      "slats":[
        {
          "detID":[500,509],
          "shape":"central",
          "nPCBs":4,
          "position":[81.2500, 0.0000, 11.7500],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":[501,508],
          "shape":"rounded",
          "nPCBs":4,
          "position":[81.2500, 37.8000, 3.7500],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":[502,507],
          "shape":"normal",
          "nPCBs":4,
          "position":[81.2500, 75.5000, 11.7500],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":[503,506],
          "shape":"normal",
          "nPCBs":3,
          "position":[61.2500, 112.8000, 3.7500],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":[504,505],
          "shape":"normal",
          "nPCBs":2,
          "position":[41.2500, 146.5000, 11.7500],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":[514,513],
          "shape":"normal",
          "nPCBs":2,
          "position":[41.2500, -146.5000, 11.7500],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":[515,512],
          "shape":"normal",
          "nPCBs":3,
          "position":[61.2500, -112.8000, 3.7500],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":[516,511],
          "shape":"normal",
          "nPCBs":4,
          "position":[81.2500, -75.5000, 11.7500],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":[517,510],
          "shape":"rounded",
          "nPCBs":4,
          "position":[81.2500, -37.8000, 3.7500],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        }
      ]
    },

    {
      "stnumber":3,
      "name":["SC06I","SC06O"],
      "moduleID":[6,7],
      "type":"slat",
      "zpos":[-998.4988,-998.5012],
      "slats":[
        {
          "detID":[600,609],
          "shape":"central",
          "nPCBs":4,
          "position":[81.2500, 0.0000, 11.7500],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":[601,608],
          "shape":"rounded",
          "nPCBs":4,
          "position":[81.2500, 37.8000, 3.7500],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":[602,607],
          "shape":"normal",
          "nPCBs":4,
          "position":[81.2500, 75.5000, 11.7500],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":[603,606],
          "shape":"normal",
          "nPCBs":3,
          "position":[61.2500, 112.8000, 3.7500],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":[604,605],
          "shape":"normal",
          "nPCBs":2,
          "position":[41.2500, 146.5000, 11.7500],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":[614,613],
          "shape":"normal",
          "nPCBs":2,
          "position":[41.2500, -146.5000, 11.7500],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":[615,612],
          "shape":"normal",
          "nPCBs":3,
          "position":[61.2500, -112.8000, 3.7500],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":[616,611],
          "shape":"normal",
          "nPCBs":4,
          "position":[81.2500, -75.5000, 11.7500],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":[617,610],
          "shape":"rounded",
          "nPCBs":4,
          "position":[81.2500, -37.8000, 3.7500],
          "rotation":[90.0000, 180.0000, 90.0000, 270.0000, 0.0000, 0.0000]
        }
      ]
    },

    {
      "stnumber":4,
      "name":["SC07I","SC07O","SC08I","SC08O"],
      "moduleID":[8,9,10,11],
      "type":"slat",
      "zpos":[-1276.4988,-1276.5012,-1307.4988,-1307.5012],
      "slats":[
        {
          "detID":[700,713,800,813],
          "shape":"normal",
          "nPCBs":5,
          "position":[140.0000, 0.0000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":[701,712,801,812],
          "shape":"rounded",
          "nPCBs":6,
          "position":[121.2500, 38.2000, 3.5000],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":[702,711,802,811],
          "shape":"normal",
          "nPCBs":5,
          "position":[101.2500, 72.6000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":[703,710,803,810],
          "shape":"normal",
          "nPCBs":5,
          "position":[101.2500, 109.2000, 3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":[704,709,804,809],
          "shape":"normal",
          "nPCBs":4,
          "position":[81.2500, 138.5000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":[705,708,805,808],
          "shape":"normal",
          "nPCBs":3,
          "position":[61.2500, 175.5000, 3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":[706,707,806,807],
          "shape":"normal",
          "nPCBs":2,
          "position":[41.2500, 204.5000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":[720,719,820,819],
          "shape":"normal",
          "nPCBs":2,
          "position":[41.2500, -204.5000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":[721,718,821,818],
          "shape":"normal",
          "nPCBs":3,
          "position":[61.2500, -175.5000, 3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":[722,717,822,817],
          "shape":"normal",
          "nPCBs":4,
          "position":[81.2500, -138.5000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":[723,716,823,816],
          "shape":"normal",
          "nPCBs":5,
          "position":[101.2500, -109.2000, 3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":[724,715,824,815],
          "shape":"normal",
          "nPCBs":5,
          "position":[101.2500, -72.6000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":[725,714,825,814],
          "shape":"rounded",
          "nPCBs":6,
          "position":[121.2500, -38.2000, 3.5000],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        }
      ]
    },

    {
      "stnumber":5,
      "name":["SC09I","SC09O","SC10I","SC10O"],
      "moduleID":[12,13,14,15],
      "type":"slat",
      "zpos":[-1406.4988,-1406.5012,-1437.4988,-1437.5012],
      "slats":[
        {
          "detID":[900,913,1000,1013],
          "shape":"normal",
          "nPCBs":5,
          "position":[140.0000, 0.0000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":[901,912,1001,1012],
          "shape":"rounded",
          "nPCBs":6,
          "position":[121.2500, 38.2000, 3.5000],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        },
        {
          "detID":[902,911,1002,1011],
          "shape":"normal",
          "nPCBs":6,
          "position":[121.2500, 72.6000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":[903,910,1003,1010],
          "shape":"normal",
          "nPCBs":6,
          "position":[121.2500, 109.2000, 3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":[904,909,1004,1009],
          "shape":"normal",
          "nPCBs":5,
          "position":[101.2500, 138.5000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":[905,908,1005,1008],
          "shape":"normal",
          "nPCBs":4,
          "position":[81.2500, 175.5000, 3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":[906,907,1006,1007],
          "shape":"normal",
          "nPCBs":3,
          "position":[61.2500, 204.5000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":[920,919,1020,1019],
          "shape":"normal",
          "nPCBs":3,
          "position":[61.2500, -204.5000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":[921,918,1021,1018],
          "shape":"normal",
          "nPCBs":4,
          "position":[81.2500, -175.5000, 3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":[922,917,1022,1017],
          "shape":"normal",
          "nPCBs":5,
          "position":[101.2500, -138.5000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":[923,916,1023,1016],
          "shape":"normal",
          "nPCBs":6,
          "position":[121.2500, -109.2000, 3.5000],
          "rotation":[90.0000, 0.0000, 90.0000, 90.0000, 0.0000, 0.0000]
        },
        {
          "detID":[924,915,1024,1015],
          "shape":"normal",
          "nPCBs":6,
          "position":[121.2500, -72.6000, 12.0000],
          "rotation":[90.0000, 0.0000, 90.0000, 270.0000, 180.0000, 0.0000]
        },
        {
          "detID":[925,914,1025,1014],
          "shape":"rounded",
          "nPCBs":6,
          "position":[121.2500, -38.2000, 3.5000],
          "rotation":[90.0000, 180.0000, 90.0000, 90.0000, 180.0000, 0.0000]
        }
      ]
    }

  ]
}
)";
} // namespace mch
} // namespace o2
