// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_SIMULATION_RADIOGRAPHER_H
#define O2_MCH_SIMULATION_RADIOGRAPHER_H

#include "MCHContour/BBox.h"
class TH2;

namespace o2
{
namespace mch
{

/// get a radlen radiograph of a given detection element within box with the given granularity
TH2* getRadio(int detElemId, o2::mch::contour::BBox<float> box, float xstep, float ystep, float thickness=5 /* cm */);

} // namespace mch
} // namespace o2 
#endif