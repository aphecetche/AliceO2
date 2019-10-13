// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_BAREDECODER_H
#define O2_MCH_RAW_BAREDECODER_H

#include "CRUDecoder.h"
#include "MCHRaw/RawDataHeaderHandler.h"
#include "MCHRaw/SampaChannelHandler.h"
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

class BareDecoder
{
 public:
  /// Constructs a decoder
  /// \param rdhHandler the handler that will be called for each RDH
  /// (Raw Data Header) that is found in the data stream
  /// \param channelHandler the handler that will be called for each
  /// piece of sampa data (a SampaCluster, i.e. a part of a time window)
  /// \param chargeSumMode is true if the sampa generating the data
  /// is in clusterSum mode. This parameter _must_ be specified and _must_
  /// match the data used, as there is no way to deduce this value from
  /// the data itself.
  BareDecoder(RawDataHeaderHandler rdhHandler, SampaChannelHandler channelHandler, bool chargeSumMode = true);

  ~BareDecoder();

  /// decode the buffer
  /// \return the number of RDH encountered
  int operator()(gsl::span<uint32_t> buffer);

 private:
  void reset();

 private:
  // FIXME: how many CRUs really ? 18 gives already 17280 elinks,
  // which is more than the number of dual sampas ?
  std::array<CRUDecoder, 18> mCruDecoders; //< helper decoders
  RawDataHeaderHandler mRdhHandler;        //< RDH handler that is called at each RDH
  uint32_t mOrbit;                         //< the current orbit the decoder is currently at
  size_t mNofOrbitSeen;                    //< the total number of orbits the decoder has seen so far
  size_t mNofOrbitJumps;                   //< the total number of orbit jumps the decoder has seen so far
};
} // namespace raw
} // namespace mch
} // namespace o2

#endif
