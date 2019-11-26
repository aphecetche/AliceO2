// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHMappingFactory/CreateSegmentation.h"
#include <map>
#include <vector>

namespace
{
std::map<int, int> createDEMap()
{
  std::map<int, int> m;
  int i{0};
  o2::mch::mapping::forEachDetectionElement([&m, &i](int deid) {
    m[deid] = i++;
  });
  return m;
}

int deId2deIndex(int detElemId)
{
  static std::map<int, int> m = createDEMap();
  return m[detElemId];
}

std::vector<o2::mch::mapping::Segmentation> createSegmentations()
{
  std::vector<o2::mch::mapping::Segmentation> segs;
  o2::mch::mapping::forEachDetectionElement([&segs](int deid) {
    segs.emplace_back(deid);
  });
  return segs;
}

} // namespace

namespace o2::mch::mapping
{

const Segmentation& segmentation(int detElemId)
{
  static auto segs = createSegmentations();
  return segs[deId2deIndex(detElemId)];
}
} // namespace o2::mch::mapping
