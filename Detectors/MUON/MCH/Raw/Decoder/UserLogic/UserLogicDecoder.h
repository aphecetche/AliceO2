// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_USERLOGICDECODER_H
#define O2_MCH_RAW_USERLOGICDECODER_H

#include "MCHRawDecoder/RawDataHeaderHandler.h"
#include "MCHRawDecoder/SampaChannelHandler.h"
#include <cstdlib>
#include <gsl/span>
namespace o2
{
namespace mch
{
namespace raw
{
/// @brief Decoder for MCH Bare Raw Data Format.
///
/// The Bare Data Format is used when no user logic
/// is present or activated in the CRU.

class UserLogicDecoder
{
 public:
  /// Constructs a decoder
  /// \param rdhHandler the handler that will be called for each RDH
  /// (Raw Data Header) that is found in the data stream
  /// \param channelHandler the handler that will be called for each
  /// piece of sampa data (a SampaCluster, i.e. a part of a time window)
  UserLogicDecoder(RawDataHeaderHandler rdhHandler, SampaChannelHandler channelHandler);

  /// decode the buffer
  /// \return the number of RDH encountered
  int operator()(gsl::span<uint8_t> buffer);

 private:
  RawDataHeaderHandler mRdhHandler;     //< RDH handler that is called at each RDH
  SampaChannelHandler mChannelHandlder; //< Sampa channel handler that is call for each sampa cluster
};
} // namespace raw
} // namespace mch
} // namespace o2

#endif
