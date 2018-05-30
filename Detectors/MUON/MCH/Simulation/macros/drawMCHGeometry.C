// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#if !defined(__CLING__) || defined(__ROOTCLING__)
#include "MCHSimulation/Geometry.h"

#include <TGeoManager.h>
#include <TObjArray.h>
#include <TObjString.h>
#include <TROOT.h>
#include <TString.h>
#include <TSystem.h>

#include <iomanip>
#include <iostream>
#endif

void createMCHGeometry()
{
  TGeoManager* g = new TGeoManager("TEST", "MCH");
  TGeoVolume* top = o2::mch::createAirVacuumCave("cave");
  g->SetTopVolume(top);
  o2::mch::createGeometry(*top);
}

void drawMCHGeometry()
{
  // minimal macro to test setup of the geometry

  createMCHGeometry();

  const TString ToHide = "cave";
  TObjArray* lToHide = ToHide.Tokenize(" ");
  TIter* iToHide = new TIter(lToHide);
  TObjString* name;
  while ((name = (TObjString*)iToHide->Next()))
    gGeoManager->GetVolume(name->GetName())->SetVisibility(kFALSE);
  TString ToShow = "SC01I SC01O SC02I SC02O SC03I SC03O SC04I SC04O SC05I SC05O SC06I SC06O SC07I SC07O SC08I SC08O SC09I SC09O SC10I SC10O";
  TObjArray* lToShow = ToShow.Tokenize(" ");
  TIter* iToShow = new TIter(lToShow);
  while ((name = (TObjString*)iToShow->Next())) {
    if (gGeoManager->GetVolume(name->GetName()))
      gGeoManager->GetVolume(name->GetName())->SetVisibility(kTRUE);
    else
      printf("No volume <%s>\n", name->GetName());
  }
  const TString ToTrans = "";
  TObjArray* lToTrans = ToTrans.Tokenize(" ");
  TIter* iToTrans = new TIter(lToTrans);
  while ((name = (TObjString*)iToTrans->Next())) {
    auto v = gGeoManager->GetVolume(name->GetName());
    if (v)
      v->SetTransparency(50);
    else
      printf("Volume %s not found ...\n", name->GetName());
  }

  gGeoManager->GetTopVolume()->Draw();
  gGeoManager->Export("MCHgeometry.root");
}
