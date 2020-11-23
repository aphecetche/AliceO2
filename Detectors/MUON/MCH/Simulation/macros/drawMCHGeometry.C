#if !defined(__CLING__) || defined(__ROOTCLING__)
#include "MCHSimulation/GeometryTest.h"
#include "MCHSimulation/Geometry.h"
#include "TGeoManager.h"
#include "TGeoPhysicalNode.h"
#include "TGeoMatrix.h"
#endif

void drawMCHGeometry()
{
  o2::mch::test::drawGeometry();
  if (!gGeoManager->IsClosed()) {
    gGeoManager->CloseGeometry();
  }
  gGeoManager->Export("mch-geometry-before-alignable.root");
  o2::mch::addAlignableVolumesMCH();
  gGeoManager->Export("mch-geometry-after-alignable.root");

}
