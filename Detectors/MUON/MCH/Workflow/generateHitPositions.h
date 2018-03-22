// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

///
/// @author  Laurent Aphecetche

#ifndef O2_MCH_GENERATEHITPOSITIONS_H
#define O2_MCH_GENERATEHITPOSITIONS_H

#include <vector>
#include "MCHMappingInterface/Segmentation.h"

namespace o2
{
namespace mch
{
namespace test
{

struct Position {
  double x, y;
};

std::vector<Position> generateHitPositions(int n, const o2::mch::mapping::Segmentation& seg);

} // namespace simulation
} // namespace mch
} // namespace o2

#endif
