// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_CRU_DECODER_H
#define O2_MCH_RAW_CRU_DECODER_H

#include "MCHRawDecoder/SampaChannelHandler.h"
#include <gsl/span>
#include <array>
#include "MakeArray.h"
#include "Assertions.h"

namespace o2
{
namespace mch
{
namespace raw
{
/// @brief A CRUDecoder manages 24 GBTDecoder objects.
///
///

template <typename GBTDECODER>
class CRUDecoder
{
 public:
  /// Constructor.
  /// \param cruId the identifier of the CRU
  /// \param sampaChannelHandler the callable that will handle the SampaCluster
  /// that gets decoded.
  explicit CRUDecoder(int cruId, SampaChannelHandler channelHandler,
                      bool chargeSumMode);

  /// decode the data in buffer, assuming it's coming from the given GBT.
  void decode(int gbtid, gsl::span<uint8_t> buffer);

  /// reset our internal GBTDecoders
  void reset();

 private:
  int mCruId;
  std::array<GBTDECODER, 24> mGbtDecoders;
};

template <typename GBTDECODER>
CRUDecoder<GBTDECODER>::CRUDecoder(int cruId,
                                   SampaChannelHandler sampaChannelHandler,
                                   bool chargeSumMode)
  : mCruId{cruId},
    mGbtDecoders{::makeArray<24>([=](size_t i) { return GBTDECODER(cruId, i, sampaChannelHandler, chargeSumMode); })}
{
}

template <typename GBTDECODER>
void CRUDecoder<GBTDECODER>::decode(int gbtId, gsl::span<uint8_t> buffer)
{
  assertIsInRange("gbtId", gbtId, 0, mGbtDecoders.size());
  if (buffer.size() % 16) {
    throw std::invalid_argument("buffer size should be a multiple of 16");
  }
  mGbtDecoders[gbtId].append(buffer);
}

template <typename GBTDECODER>
void CRUDecoder<GBTDECODER>::reset()
{
  for (auto& g : mGbtDecoders) {
    g.reset();
  }
}
} // namespace raw
} // namespace mch
} // namespace o2
#endif
