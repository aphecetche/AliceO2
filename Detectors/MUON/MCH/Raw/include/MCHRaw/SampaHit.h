// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_SAMPA_HIT_H
#define O2_MCH_RAW_SAMPA_HIT_H

#include <cstdlib>
#include <vector>

namespace o2
{
namespace mch
{
namespace raw
{

struct SampaHit { // aka Sampa Cluster in the Sampa documentation
  uint8_t cruId;
  uint8_t linkId;
  uint8_t chip;
  uint8_t channel;
  uint16_t timestamp;
  uint32_t chargeSum;
  std::vector<uint16_t> samples;

  uint16_t nofSamples() const;
  bool isClusterSum() const;
};

} // namespace raw
} // namespace mch
} // namespace o2
#endif
