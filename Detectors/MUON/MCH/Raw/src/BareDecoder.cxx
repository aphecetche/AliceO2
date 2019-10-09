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
namespace o2
{
namespace mch
{
namespace raw
{

BareDecoder::BareDecoder(RawDataHeaderHandler rdhHandler, SampaChannelHandler channelHandler) : mRdhHandler(rdhHandler), mCruDecoders{::makeArray<18>([=](size_t i) { return CRUDecoder(i, channelHandler); })}
{
}

int BareDecoder::operator()(gsl::span<uint32_t> buffer)
{
  RAWDataHeader rdh;
  int index{0};
  int nofRDHs{0};

  while (index < buffer.size()) {
    rdh = createRDH(buffer.subspan(index, sizeof(rdh) / 4));
    if (!isValid(rdh)) {
      break;
    }
    ++nofRDHs;
    int payloadSize = rdh.memorySize - sizeof(rdh);
    bool shouldDecode = mRdhHandler(rdh);
    if (shouldDecode) {
      size_t n = static_cast<size_t>(payloadSize) / 4;
      size_t pos = static_cast<size_t>(index + sizeof(rdh) / 4);
      mCruDecoders[rdh.cruId].decode(rdh.linkId,
                                     buffer.subspan(pos, n));
    }
    index += rdh.offsetNextPacket / 4;
  }
  return nofRDHs;
}
} // namespace raw
} // namespace mch
} // namespace o2
