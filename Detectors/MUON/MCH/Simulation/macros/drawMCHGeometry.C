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
#include "DetectorsPassive/Cave.h"
#include "DetectorsPassive/FrameStructure.h"
#include "MCHSimulation/Detector.h"
#include "FairRunSim.h"
#include "TGeoManager.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TROOT.h"
#include "TString.h"
#include "TSystem.h"

#include "MCHSimulation/SlatGeometry.h"

#include <boost/program_options.hpp>
#include <iomanip>
#include <iostream>
#endif

void drawMCHGeometry()
{
  // minimal macro to test setup of the geometry

  TString dir = getenv("VMCWORKDIR");
  TString geom_dir = dir + "/Detectors/Geometry/";
  gSystem->Setenv("GEOMPATH", geom_dir.Data());

  TString tut_configdir = dir + "/Detectors/gconfig";
  gSystem->Setenv("CONFIG_DIR", tut_configdir.Data());

  // Create simulation run
  FairRunSim* run = new FairRunSim();
  run->SetOutputFile("foo.root"); // Output file
  run->SetName("TGeant3");        // Transport engine
  // Create media
  run->SetMaterials("media.geo"); // Materials

  // Create geometry

  o2::Passive::Cave* cave = new o2::Passive::Cave("CAVE");
  cave->SetGeometryFileName("cave.geo");
  run->AddModule(cave);
/*
  o2::passive::FrameStructure* frame = new o2::passive::FrameStructure("Frame", "Frame");
  run->AddModule(frame);
*/
  o2::mch::Detector* mch = new o2::mch::Detector(kTRUE);
  run->AddModule(mch);

  run->Init();

  {
    const TString ToHide = "cave";
    TObjArray* lToHide = ToHide.Tokenize(" ");
    TIter* iToHide = new TIter(lToHide);
    TObjString* name;
    while ((name = (TObjString*)iToHide->Next()))
      gGeoManager->GetVolume(name->GetName())->SetVisibility(kFALSE);
    TString ToShow = "SC05I SC05O SC06I SC06O SC07I SC07O SC08I SC08O SC09I SC09O SC10I SC10O";
    TObjArray* lToShow = ToShow.Tokenize(" ");
    TIter* iToShow = new TIter(lToShow);
    while ((name = (TObjString*)iToShow->Next())){
      if(gGeoManager->GetVolume(name->GetName()))
         gGeoManager->GetVolume(name->GetName())->SetVisibility(kTRUE);
      else
        printf("No volume <%s>\n",name->GetName()) ;
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
  }

  gGeoManager->GetListOfVolumes()->ls();

  gGeoManager->GetTopVolume()->Draw();
  gGeoManager->Export("MCHgeometry.root");
}
