// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction. 
/// get the local-to-global transformation for a given detection element

#ifndef O2_MCH_BASE_GEO_TRANSFORMATIONS_H
#define O2_MCH_BASE_GEO_TRANSFORMATIONS_H

#include "MathUtils/Cartesian.h"

class TGeoManager;

namespace o2::mch::geo {

/* Get a 3D transformation that can be used
 * to convert coordinates from local (x,y in detection element plane) to global
 * (x,y,z in Alice Coordinate System) and vice-versa
 * 
 * @param detElemId must be a valid detection element
 * @param geo a reference to a GeoManager that must contain MCH volumes
 *
 * @throw if detElemId is not valid
 */
o2::math_utils::Transform3D transformation(int detElemId, const TGeoManager& geo);

}

#endif
