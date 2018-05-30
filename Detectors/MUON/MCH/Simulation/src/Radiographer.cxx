// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHSimulation/Radiographer.h"

#include "DetectorsBase/GeometryManager.h"
#include "MCHSimulation/Geometry.h"
#include "MCHSimulation/GeometryTransformer.h"
#include "Math/GenVector/Cartesian3D.h"
#include "TGeoManager.h"
#include "TH2F.h"

namespace o2
{
namespace mch
{

o2::Base::GeometryManager::MatBudget getMatBudget(const o2::Transform3D& t, Vector3D<double>& n, float x, float y, float thickness)
{
  Point3D<double> point;
  t.LocalToMaster(Point3D<double>{x,y,0}, point);
  auto front = Point3D<double>(point + n * thickness / 2.0);
  auto back = Point3D<double>(point - n * thickness / 2.0);
  return o2::Base::GeometryManager::MeanMaterialBudget(front, back);
}

std::ostream& operator<<(std::ostream& os, o2::Base::GeometryManager::MatBudget m)
{
  os << "L=" << m.length << " <Rho>=" << m.meanRho << " <A>=" << m.meanA
     << " <Z>=" << m.meanZ << " <x/x0>=" << m.meanX2X0 << " nCross=" << m.nCross;
  return os;
}

Vector3D<double> getNormalVector(const o2::Transform3D& t)
{
  Point3D<double> pa,pb;
  t.LocalToMaster(Point3D<double>{0,1,0}, pb);
  t.LocalToMaster(Point3D<double>{1,0,0}, pa);
  Vector3D<double> a{ pa };
  Vector3D<double> b{ pb };
  return a.Cross(b).Unit();
}

TH2* getRadio(int detElemId, o2::mch::contour::BBox<float> box, float xstep, float ystep, float thickness)
{
  TH2* hmatb = new TH2F("hmatb", "hmatb", (int)(box.width() / xstep), box.xmin(), box.xmax(), (int)(box.height() / ystep), box.ymin(), box.ymax());

  auto t = o2::mch::getTransformation(detElemId, *gGeoManager);

  auto normal = getNormalVector(t);

  std::cout << getMatBudget(t, normal,30, 15, 5) << "\n";
  std::cout << getMatBudget(t, normal,30, 15, 10) << "\n";
  std::cout << getMatBudget(t, normal,30, 15, 15) << "\n";
  std::cout << getMatBudget(t, normal,30, 15, 35) << "\n";

  for (auto x = box.xmin(); x < box.xmax(); x += xstep) {
    for (auto y = box.ymin(); y < box.ymax(); y += ystep) {
      auto matb = getMatBudget(t,normal, x, y, thickness);
      if (std::isfinite(matb.meanX2X0)) {
        hmatb->Fill(x, y, matb.meanX2X0);
      }
    }
  }
  return hmatb;
}
} // namespace mch
} // namespace o2