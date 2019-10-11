// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_SAMPA_CLUSTER_H
#define O2_MCH_RAW_SAMPA_CLUSTER_H

#include <cstdlib>
#include <vector>
#include <iostream>

namespace o2
{
namespace mch
{
namespace raw
{

struct SampaCluster {

  explicit SampaCluster(uint16_t timestamp, uint32_t chargeSum);
  SampaCluster(uint16_t timestamp, const std::vector<uint16_t>& samples);

  uint16_t nofSamples() const;

  bool isClusterSum() const;

  uint16_t nof10BitWords() const;

  uint16_t timestamp;
  uint32_t chargeSum;
  std::vector<uint16_t> samples;
};

std::ostream& operator<<(std::ostream& os, const SampaCluster& sc);
} // namespace raw
} // namespace mch
} // namespace o2
#endif
