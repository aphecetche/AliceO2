// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "UserLogicDecoder.h"
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

UserLogicDecoder::UserLogicDecoder(RawDataHeaderHandler rdhHandler,
                                   SampaChannelHandler channelHandler)
  : mRdhHandler(rdhHandler)
{
}

int UserLogicDecoder::operator()(gsl::span<uint8_t> buffer)
{
  RAWDataHeader rdh;
  const size_t nofRDHWords = sizeof(rdh);
  int index{0};
  int nofRDHs{0};

  while (index < buffer.size()) {
    rdh = createRDH(buffer.subspan(index, nofRDHWords));
    if (!isValid(rdh)) {
      break;
    }
    ++nofRDHs;
    // mOrbit = rdhOrbit(rdh);
    int payloadSize = rdhPayloadSize(rdh);
    bool shouldDecode = mRdhHandler(rdh);
    if (shouldDecode) {
      size_t n = static_cast<size_t>(payloadSize);
      size_t pos = static_cast<size_t>(index + nofRDHWords);
      // mCruDecoders[rdh.cruId].decode(rdh.linkId, buffer.subspan(pos, n));
    }
    index += rdh.offsetNextPacket;
  }
  return nofRDHs;
}

} // namespace raw
} // namespace mch
} // namespace o2
