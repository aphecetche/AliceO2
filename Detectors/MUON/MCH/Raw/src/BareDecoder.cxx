// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "BareDecoder.h"
#include "MakeArray.h"
#include "MCHRaw/RAWDataHeader.h"
#include <iostream>
#include <fmt/format.h>

namespace o2
{
namespace mch
{
namespace raw
{

BareDecoder::BareDecoder(RawDataHeaderHandler rdhHandler,
                         SampaChannelHandler channelHandler,
                         bool chargeSumMode) : mRdhHandler(rdhHandler),
                                               mCruDecoders{::makeArray<18>([=](size_t i) { return CRUDecoder(i, channelHandler, chargeSumMode); })},
                                               mOrbit{0},
                                               mNofOrbitSeen{0},
                                               mNofOrbitJumps{0}
{
}

bool hasOrbitJump(uint32_t orb1, uint32_t orb2)
{
  return std::abs(static_cast<long int>(orb1 - orb2)) > 1;
}

BareDecoder::~BareDecoder()
{
  std::cout << "Nof orbits seen : " << mNofOrbitSeen << "\n";
  std::cout << "Nof orbits jumps: " << mNofOrbitJumps << "\n";
}

int BareDecoder::operator()(gsl::span<uint32_t> buffer)
{
  RAWDataHeader rdh;
  const size_t byteTo32BitWords{4};
  const size_t nofRDHWords = sizeof(rdh) / byteTo32BitWords;
  int index{0};
  int nofRDHs{0};

  while (index < buffer.size()) {
    rdh = createRDH(buffer.subspan(index, nofRDHWords));
    if (!isValid(rdh)) {
      break;
    }
    ++nofRDHs;
    if (hasOrbitJump(rdhOrbit(rdh), mOrbit)) {
      ++mNofOrbitJumps;
      reset();
    } else if (rdhOrbit(rdh) != mOrbit) {
      ++mNofOrbitSeen;
    }
    mOrbit = rdhOrbit(rdh);
    int payloadSize = rdhPayloadSize(rdh);
    bool shouldDecode = mRdhHandler(rdh);
    if (shouldDecode) {
      size_t n = static_cast<size_t>(payloadSize) / byteTo32BitWords;
      size_t pos = static_cast<size_t>(index + nofRDHWords);
      mCruDecoders[rdh.cruId].decode(rdh.linkId, buffer.subspan(pos, n));
    }
    index += rdh.offsetNextPacket / byteTo32BitWords;
  }
  return nofRDHs;
} // namespace raw

void BareDecoder::reset()
{
  for (auto& c : mCruDecoders) {
    c.reset();
  }
}

} // namespace raw
} // namespace mch
} // namespace o2
