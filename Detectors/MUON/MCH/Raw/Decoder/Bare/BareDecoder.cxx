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
#include "Headers/RAWDataHeader.h"

extern std::ostream& operator<<(std::ostream&, const o2::header::RAWDataHeaderV4&);

namespace o2
{
namespace mch
{
namespace raw
{

template <typename RDH>
Decoder createBareDecoder(RawDataHeaderHandler<RDH> rdhHandler,
                          SampaChannelHandler channelHandler,
                          bool chargeSumMode)

{
  return BareDecoder<RDH>(rdhHandler, channelHandler, chargeSumMode);
}

template <typename RDH>
BareDecoder<RDH>::BareDecoder(RawDataHeaderHandler<RDH> rdhHandler,
                              SampaChannelHandler channelHandler,
                              bool chargeSumMode) : mRdhHandler(rdhHandler),
                                                    mCruDecoders{::makeArray<18>([=](size_t i) { return BareCRUDecoder(i, channelHandler, chargeSumMode); })},
                                                    mOrbit{0},
                                                    mNofOrbitSeen{0},
                                                    mNofOrbitJumps{0}
{
}

bool hasOrbitJump(uint32_t orb1, uint32_t orb2)
{
  return std::abs(static_cast<long int>(orb1 - orb2)) > 1;
}

template <typename RDH>
BareDecoder<RDH>::~BareDecoder()
{
  std::cout << "Nof orbits seen : " << mNofOrbitSeen << "\n";
  std::cout << "Nof orbits jumps: " << mNofOrbitJumps << "\n";
}

using ::operator<<;
template <typename RDH>
int BareDecoder<RDH>::operator()(gsl::span<uint8_t> buffer)
{
  RDH rdh;
  const size_t nofRDHWords = sizeof(rdh);
  int index{0};
  int nofRDHs{0};

  while (index < buffer.size()) {
    rdh = createRDH<RDH>(buffer.subspan(index, nofRDHWords));
    if (!isValid(rdh)) {
      std::cout << "Got an invalid RDH\n";
      std::cout << rdh << "\n";
      dumpBuffer(buffer.subspan(index, nofRDHWords));
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
      size_t n = static_cast<size_t>(payloadSize);
      size_t pos = static_cast<size_t>(index + nofRDHWords);
      mCruDecoders[rdh.cruID].decode(rdh.linkID, buffer.subspan(pos, n));
    }
    index += rdh.offsetToNext;
  }
  return nofRDHs;
}

template <typename RDH>
void BareDecoder<RDH>::reset()
{
  for (auto& c : mCruDecoders) {
    c.reset();
  }
}

// explicitely force (only) template specializations we want

template class BareDecoder<o2::header::RAWDataHeaderV4>;

template Decoder createBareDecoder(RawDataHeaderHandler<o2::header::RAWDataHeaderV4> rdhHandler,
                                   SampaChannelHandler channelHandler,
                                   bool chargeSumMode);

} // namespace raw
} // namespace mch
} // namespace o2
