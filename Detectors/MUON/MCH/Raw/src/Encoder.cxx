// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHRaw/Encoder.h"
#include "MCHRaw/SampaHeader.h"
#include <iostream>
#include <fmt/printf.h>

namespace
{

int computeHamming(uint64_t header)
{
  return 0;
}
} // namespace

namespace o2::mch::raw
{

namespace
{
void show(const BitSet& bs, int a, int b)
{
  auto s = bs.subset(a, a + b - 1);
  fmt::printf("%s = %d = %x\n", s.stringLSBLeft(), s.uint64(0, b - 1),
              s.uint64(0, b - 1));
}

} // namespace

// void dump(BitSet& bs, int start, int size)
// {
//   int n{0};
//   while (n < size) {
//     std::cout << SampaHeader(bs.subset(n + start, n + start + 49).uint64(0, 49));
//     show(bs, n + start + 50, 10);
//     show(bs, n + start + 60, 10);
//     show(bs, n + start + 70, 20);
//     std::cout << "\n";
//     n += 90;
//   }
// }
//
void Encoder::appendOneDualSampa(BitSet& bs, int dsid, int timestamp, gsl::span<int> channels, gsl::span<int> adcs)
{
  static int n50{0};

  // for the moment assume dsid==chipAddress and chid==channelAddress
  // but really here will have to convert (dsid,chid) => (chipAddress,channelAddress)
  // where channels is {chid}

  if (channels.size() != adcs.size()) {
    throw std::invalid_argument("channels and adcs arrays should have the same size");
  }

  SampaHeader h;
  h.chipAddress(dsid);
  h.bunchCrossingCounter(0x80000 + dsid);

  auto ix = bs.len();

  for (int i = 0; i < channels.size(); i++) {

    int s = bs.len();

    h.packetType(SampaPacketType::Data);
    h.channelAddress(channels[i]);

    h.nof10BitWords(4);
    bs.append(h.uint64(), 50);
    n50++;

    uint16_t nofSamples = 1;
    uint16_t timestamp = 100 + i;
    uint32_t chargeSum = static_cast<uint32_t>(adcs[i]);

    bs.append(nofSamples, 10);
    bs.append(timestamp, 10);
    bs.append(chargeSum, 20);
  }
  std::cout << h << "N50=" << n50 << "\n";
}

} // namespace o2::mch::raw
