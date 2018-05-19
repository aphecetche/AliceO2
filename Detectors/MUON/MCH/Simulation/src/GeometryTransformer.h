// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_GEOMETRYTRANSFORMER_H
#define O2_MCH_GEOMETRYTRANSFORMER_H

#include "MathUtils/Cartesian3D.h"

class TGeoManager;

namespace o2
{
namespace mch
{
class DECoordinateTransformer
{
 public:
  Point3D<double> localToGlobal(const Point3D<double>& local);
  Point3D<double> GlobalToLocal(const Point3D<double>& global);
};

DECoordinateTransformer getTransformer(int detElemId, const TGeoManager& geo);

} // namespace mch
} // namespace o2

#endif