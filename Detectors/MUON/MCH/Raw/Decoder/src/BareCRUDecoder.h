// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_BARE_CRU_DECODER_H
#define O2_MCH_RAW_BARE_CRU_DECODER_H

#include "MCHRawDecoder/SampaChannelHandler.h"
#include <gsl/span>
#include <array>
#include "BareGBTDecoder.h"

namespace o2
{
namespace mch
{
namespace raw
{
/// @brief A BareCRUDecoder manages 24 GBTDecoder objects.
///
/// It is used when there is no User Logic in the CRU.
///

class BareCRUDecoder
{
 public:
  /// Constructor.
  /// \param cruId the identifier of the CRU
  /// \param sampaChannelHandler the callable that will handle the SampaCluster
  /// that gets decoded.
  explicit BareCRUDecoder(int cruId, SampaChannelHandler channelHandler,
                          bool chargeSumMode);

  /// decode the data in buffer, assuming it's coming from the given GBT.
  void decode(int gbtid, gsl::span<uint8_t> buffer);

  /// reset our internal GBTDecoders
  void reset();

 private:
  int mCruId;
  std::array<BareGBTDecoder, 24> mGbtDecoders;
};

} // namespace raw
} // namespace mch
} // namespace o2
#endif
