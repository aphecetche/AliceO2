// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_PAYLOAD_DECODER_H
#define O2_MCH_RAW_PAYLOAD_DECODER_H

#include "BareGBTDecoder.h"
#include "CRUDecoder.h"
#include "DumpBuffer.h"
#include "Headers/RAWDataHeader.h"
#include "MCHRawDecoder/Decoder.h"
#include "MakeArray.h"
#include "PayloadDecoder.h"
#include "UserLogicGBTDecoder.h"
#include <cstdlib>
#include <fmt/format.h>
#include <gsl/span>
#include <iostream>

namespace o2
{
namespace mch
{
namespace raw
{
/// @brief Decoder for MCH  Raw Data Format.

constexpr int MAX_NOF_CRUS{33};

template <typename RDH, typename CRUDECODER>
class PayloadDecoder
{
 public:
  /// Constructs a decoder
  /// \param rdhHandler the handler that will be called for each RDH
  /// (Raw Data Header) that is found in the data stream
  /// \param channelHandler the handler that will be called for each
  /// piece of sampa data (a SampaCluster, i.e. a part of a time window)
  PayloadDecoder(SampaChannelHandler channelHandler);

  /// decode the buffer
  /// \return the number of RDH encountered
  size_t process(const RDH& rdh, gsl::span<uint8_t> buffer);

  void reset();

 private:
  std::array<CRUDECODER, MAX_NOF_CRUS> mCruDecoders; //< helper decoders
};

template <typename RDH, typename CRUDECODER>
PayloadDecoder<RDH, CRUDECODER>::PayloadDecoder(
  SampaChannelHandler channelHandler) : mCruDecoders{impl::makeArray<MAX_NOF_CRUS>([=](size_t i) { return CRUDECODER(i, channelHandler); })}
{
}

template <typename RDH, typename CRUDECODER>
size_t PayloadDecoder<RDH, CRUDECODER>::process(const RDH& rdh, gsl::span<uint8_t> buffer)
{
  return mCruDecoders[rdh.cruID].decode(rdhLinkId(rdh), buffer);
}

template <typename RDH, typename CRUDECODER>
void PayloadDecoder<RDH, CRUDECODER>::reset()
{
  for (auto& c : mCruDecoders) {
    c.reset();
  }
}

} // namespace raw
} // namespace mch
} // namespace o2

#endif
