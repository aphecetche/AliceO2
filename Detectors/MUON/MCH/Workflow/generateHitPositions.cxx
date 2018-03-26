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

#include <random>
#include "generateHitPositions.h"
#include "MCHMappingSegContour/SegmentationContours.h"

namespace o2
{
namespace mch
{
namespace test
{

/// Generates n (x,y) positions randomly distributed on the surface of the segmentation
std::vector<Position> generateHitPositions(long n, const o2::mch::mapping::Segmentation& seg)
{
  std::random_device rd;
  std::mt19937 mt(rd());
  std::vector<Position> hitPositions;

  hitPositions.resize(n);

  auto bbox = o2::mch::mapping::getBBox(seg);
  std::uniform_real_distribution<double> distX{ bbox.xmin(), bbox.xmax() };
  std::uniform_real_distribution<double> distY{ bbox.ymin(), bbox.ymax() };

  while (hitPositions.size() < n) {
    auto x = distX(mt);
    auto y = distY(mt);
    if (seg.isValid(seg.findPadByPosition(x,y))) {
      hitPositions.push_back({x,y});
    }
  }
  return hitPositions;
}

} // namespace test
} // namespace mch
} // namespace o2
