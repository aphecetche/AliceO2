// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHRawCommon/SampaCluster.h"
#include "MCHRawCommon/DataFormats.h"
#include "Assertions.h"
#include <fmt/format.h>

namespace o2
{
namespace mch
{
namespace raw
{

SampaCluster::SampaCluster(uint16_t timestamp, uint32_t chargeSum)
  : timestamp(impl::assertIsInRange("timestamp", timestamp, 0, 0x3FF)),   // 10 bits
    chargeSum(impl::assertIsInRange("chargeSum", chargeSum, 0, 0xFFFFF)), // 20 bits
    samples{}

{
}

SampaCluster::SampaCluster(uint16_t timestamp, const std::vector<uint16_t>& samples)
  : timestamp(impl::assertIsInRange("timestamp", timestamp, 0, 0x3FF)),
    chargeSum(0),
    samples(samples.begin(), samples.end())
{
  if (samples.empty()) {
    throw std::invalid_argument("cannot add data with no sample");
  }
  for (auto i = 0; i < samples.size(); i++) {
    impl::assertIsInRange(fmt::format("samples[{}]", i), samples[i], 0, 0x3FF);
  }
}

uint16_t SampaCluster::nofSamples() const
{
  if (!samples.empty()) {
    return samples.size();
  }
  return 1;
}

bool SampaCluster::isClusterSum() const
{
  return samples.empty();
}

uint16_t SampaCluster::nof10BitWords() const
{
  uint16_t n10{2}; // 10 bits (nsamples) + 10 bits (timestamp)
  if (isClusterSum()) {
    n10 += 2; // 20 bits (chargesum)
  } else {
    for (auto s : samples) {
      ++n10; // 10 bits for each sample
    }
  }
  return n10;
}

std::string asString(const SampaCluster& sc)
{
  std::string s = fmt::format("ts {:4d} ", sc.timestamp);
  if (sc.isClusterSum()) {
    s += fmt::format("q {:6d}", sc.chargeSum);
  } else {
    s += fmt::format("n {:4d} q [ ", sc.samples.size());
    for (auto sa : sc.samples) {
      s += fmt::format("{:4d} ", sa);
    }
    s += "]";
  }
  return s;
}

std::ostream& operator<<(std::ostream& os, const SampaCluster& sc)
{
  os << asString(sc);
  return os;
}

template <>
std::vector<SampaCluster> createSampaClusters<raw::ChargeSumMode>(uint16_t ts, float adc)
{
  return {raw::SampaCluster(ts, static_cast<uint32_t>(adc))};
}

template <>
std::vector<SampaCluster> createSampaClusters<raw::SampleMode>(uint16_t ts, float adc)
{
  uint32_t a = static_cast<uint32_t>(adc);
  std::vector<uint16_t> samples;

  samples.emplace_back(static_cast<uint16_t>((a & 0xFFC00) >> 10));
  samples.emplace_back(static_cast<uint16_t>(a & 0x3FF));
  return {raw::SampaCluster(ts, samples)};
}

} // namespace raw
} // namespace mch
} // namespace o2
