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
#include "TGeoManager.h"
#include "TString.h"
#include "TSystem.h"

#include <DetectorsBase/Detector.h>
#include <Field/MagneticField.h>
#include <SimConfig/SimConfig.h>
#include "FairRunSim.h"
#include <FairLogger.h>
#include <algorithm>
#include "TMethodCall.h"
#endif

namespace o2
{
namespace Base
{

namespace
{

/// TODO: this map would be better created from bottom-up
/// (e.g. each detector implementation registering itself the det<->classname relationship)
/// instead of top-to-bottom as here.
///
/// But it allows easy testing now.
const std::map<std::string, std::string> det2class{
  { "CAVE", "o2::Passive::Cave" },   { "ABSO", "o2::passive::Absorber" },
  { "DIPO", "o2::passive::Dipole" }, { "FRAME", "o2::passive::FrameStructure" },
  { "HALL", "o2::passive::Hall" },   { "MAG", "o2::passive::Magnet" },
  { "PIPE", "o2::passive::Pipe" },   { "SHIL", "o2::passive::Shil" },
  { "EMC", "o2::EMCAL::Detector" },  { "FIT", "o2::fit::Detector" },
  { "ITS", "o2::ITS::Detector" },    { "MFT", "o2::MFT::Detector" },
  { "PHS", "o2::phos::Detector" },   { "TOF", "o2::tof::Detector" },
  { "TPC", "o2::TPC::Detector" },    { "TRD", "o2::trd::Detector" }
};

std::string constructorNameFromFullyQualifiedName(const char* className)
{
  TString sClassName{ className };
  auto i = sClassName.Last(':');
  return { sClassName(i + 1, sClassName.Length() - i).Data() };
}
} // namespace

/// Create a FairModule object from the {moduleName,className,isActive} triplet.
///
/// We use the fact that all FairModules are Root classes in order to use
/// the TMethodCall mechanism to create the object.
///
/// There's mostly two types of constructors for o2::Base::Detector daughter classes:
///
/// - using two strings (name,title) for passive detectors
/// - using one bool (isActive) for sensitive detectors
///
/// we try to automatically find out here which one to use
///
/// \param moduleName is one of the detector or passive module name (TODO: for the detector name,
/// should probably enforce it is one of o2::detectors::DetID ?)
/// \param className the fully qualified (i.e. with namespaces) classname to be instanciated
/// \param isActive for detector, whether or not hits should be created (TODO: check that is indeed the intent of this
/// parameter, in which case all detectors should offer it in their ctor (not the case for e.g. MFT for the moment)
/// \return a FairModule pointer or nullptr if creation failed
///
std::unique_ptr<FairModule> createFairModule(const char* moduleName, const char* className, bool isActive)
{
  TClass* c = TClass::GetClass(className);

  if (!c) {
    LOG(ERROR) << "Cannot get class " << className;
    return nullptr;
  }

  int npars{ -1 };

  TMethodCall call;

  auto ctorName{ constructorNameFromFullyQualifiedName(className) };

  // try with 2 strings ctor first
  call.InitWithPrototype(c, ctorName.c_str(), "const char*, const char*");

  if (call.IsValid()) {
    npars = 2;
  } else {
    // then with 1 bool ctor
    call.InitWithPrototype(c, ctorName.c_str(), "Bool_t");

    if (call.IsValid()) {
      npars = 1;
    }
  }

  if (npars < 0) {
    LOG(ERROR) << "Could not find a suitable constructor for class " << className;
    return nullptr;
  }

  Long_t returnLong(0);

  if (npars == 1) {
    Long_t params[] = { (Long_t)(isActive) };
    call.SetParamPtrs((void*)(params), 1);
    call.Execute((void*)(nullptr), returnLong);
  } else if (npars == 2) {
    // for the (name,title) constructor we use moduleName as name, and assume there is a sensible
    // default for title...
    Long_t params[] = { (Long_t)(moduleName) };
    call.SetParamPtrs((void*)(params), 1);
    call.Execute((void*)(nullptr), returnLong);
  }

  return std::unique_ptr<FairModule>(reinterpret_cast<FairModule*>(returnLong));
}

} // namespace Base
} // namespace o2

void finalize_geometry(FairRunSim* run);

// decides whether the FairModule named 's' has been
// requested in the configuration (or is required
// for some reason -cavern or frame-)
bool isActivated(std::string s)
{
  if (s == "CAVE") {
    // we always need the cavern
    return true;
  }

  // access user configuration for list of wanted modules
  const auto& modulelist = o2::conf::SimConfig::Instance().getActiveDetectors();
  auto active = (std::find(modulelist.begin(), modulelist.end(), s) != modulelist.end());

  if (s == "FRAME") {
    // the frame structure must be present to support other detectors
    return active || isActivated("TOF") || isActivated("TRD");
  }

  return active;
}

// create a number of FairModules, either active or passive
void createModules(FairRunSim& runner, const std::vector<std::string>& modules, bool isActive)
{
  for (auto& moduleName : modules) {
    auto it = o2::Base::det2class.find(moduleName);
    if (it==o2::Base::det2class.end()) {
      throw std::runtime_error("Don't know which class to use for module " + moduleName);
    }
    if (isActivated(moduleName)) {
      auto module = o2::Base::createFairModule(moduleName.c_str(),it->second.c_str(), isActive);
      if (!module) {
        throw std::runtime_error("Could not create module " + moduleName);
      }
      runner.AddModule(module.release());
    }
  }
}

// a "factory" like macro to instantiate the O2 geometry
void build_geometry(FairRunSim* run = nullptr)
{
  bool geomonly = (run == nullptr);

  // minimal macro to test setup of the geometry

  TString dir = getenv("VMCWORKDIR");
  TString geom_dir = dir + "/Detectors/Geometry/";
  gSystem->Setenv("GEOMPATH", geom_dir.Data());

  TString tut_configdir = dir + "/Detectors/gconfig";
  gSystem->Setenv("CONFIG_DIR", tut_configdir.Data());

  // Create simulation run if it does not exist
  if (run == nullptr) {
    run = new FairRunSim();
    run->SetOutputFile("foo.root"); // Output file
    run->SetName("TGeant3");        // Transport engine
  }
  // Create media
  run->SetMaterials("media.geo"); // Materials

  // we need a field to properly init the media
  auto field = new o2::field::MagneticField("Maps", "Maps", -1., -1., o2::field::MagFieldParam::k5kG);
  run->SetField(field);

  // Create geometry

  bool isActive{ true };

  createModules(*run, { "CAVE", "ABSO", "DIPO", "FRAME", "HALL", "MAG", "PIPE", "SHIL" }, !isActive);
  createModules(*run, { "EMC", "FIT", "ITS", "MFT", "PHS", "TOF", "TPC", "TRD" }, isActive);

  if (geomonly) {
    run->Init();
    finalize_geometry(run);
    gGeoManager->Export("O2geometry.root");
  }
}

void finalize_geometry(FairRunSim* run)
{
  // finalize geometry and declare alignable volumes
  // this should be called geometry is fully built

  if (!gGeoManager) {
    LOG(ERROR) << "gGeomManager is not available" << FairLogger::endl;
    return;
  }

  gGeoManager->CloseGeometry();
  if (!run) {
    LOG(ERROR) << "FairRunSim is not available" << FairLogger::endl;
    return;
  }

  const TObjArray* modArr = run->GetListOfModules();
  TIter next(modArr);
  FairModule* module = nullptr;
  while ((module = (FairModule*)next())) {
    o2::Base::Detector* det = dynamic_cast<o2::Base::Detector*>(module);
    if (det)
      det->addAlignableVolumes();
  }
}

