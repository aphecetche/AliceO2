// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_ENCODER_GBT_ENCODER_H
#define O2_MCH_RAW_ENCODER_GBT_ENCODER_H

#include <array>
#include <vector>
#include <cstdlib>
#include "MCHRawCommon/SampaCluster.h"
#include "MCHRawCommon/DataFormats.h"
#include <functional>
#include <fmt/printf.h>
#include <stdexcept>
#include "MakeArray.h"
#include "Assertions.h"
#include "MoveBuffer.h"
#include "ElinkEncoder.h"
#include "ElinkEncoderMerger.h"

namespace o2::mch::raw
{

/// @brief A GBTEncoder manages 40 ElinkEncoder to encode the data of one GBT.
///
/// Channel data is added using the addChannelData() method.
/// The encoded data (in the form of 64 bits words)
/// is exported to 8-bits words buffer using the moveToBuffer() method.
///
/// \nosubgrouping

template <typename FORMAT, typename CHARGESUM>
class GBTEncoder
{
 public:
  /// Constructor.
  /// \param cruId the CRU this GBT is part of
  /// \param gbtId the id of this GBT. _Must_ be between 0 and 23
  /// or an exception is thrown
  GBTEncoder(int cruId, int gbtId);

  /** @name Main interface.
    */
  ///@{
  /// add data for one channel.
  ///
  /// \param elinkId 0..39
  /// \param chId 0..31
  /// \param data vector of SampaCluster objects
  void addChannelData(uint8_t elinkId, uint8_t chId, const std::vector<SampaCluster>& data);

  /// reset local bunch-crossing counter.
  ///
  /// (the one that is used in the sampa headers)
  void resetLocalBunchCrossing();

  /// Export our encoded data.
  ///
  /// The internal GBT words that have been accumulated so far are
  /// _moved_ (i.e. deleted from this object) to the external buffer of bytes.
  /// Returns the number of bytes added to buffer
  size_t moveToBuffer(std::vector<uint8_t>& buffer);
  ///@}

  /** @name Methods for testing.
    */
  ///@{
  /// Sets to true to bypass simulation of time misalignment of elinks.
  static bool forceNoPhase;
  ///@}

  /// returns the GBT id
  int id() const { return mGbtId; }

 private:
  int mCruId;                                              //< CRU this GBT belongs to
  int mGbtId;                                              //< id of this GBT (0..23)
  std::array<ElinkEncoder<FORMAT, CHARGESUM>, 40> mElinks; //< the 40 Elinks we manage
  std::vector<uint64_t> mGbtWords;                         //< the GBT words (each GBT word of 80 bits is represented by 2 64 bits words) we've accumulated so far
  ElinkEncoderMerger<FORMAT, CHARGESUM> mElinkMerger;
};

inline int phase(int i, bool forceNoPhase)
{
  // generate the phase for the i-th ElinkEncoder
  // the default value of -1 means it will be random and decided
  // by the ElinkEncoder ctor
  //
  // if > 0 it will set a fixed phase at the beginning of the life
  // of the ElinkEncoder
  //
  // returning zero will simply disable the phase

  if (forceNoPhase) {
    return 0;
  }
  return -1;
}

template <typename FORMAT, typename CHARGESUM>
bool GBTEncoder<FORMAT, CHARGESUM>::forceNoPhase = false;

template <typename FORMAT, typename CHARGESUM>
GBTEncoder<FORMAT, CHARGESUM>::GBTEncoder(int cruId, int linkId)
  : mCruId(cruId),
    mGbtId(linkId),
    mElinks{impl::makeArray<40>([](size_t i) { return ElinkEncoder<FORMAT, CHARGESUM>(i, phase(i, GBTEncoder<FORMAT, CHARGESUM>::forceNoPhase)); })},
    mGbtWords{},
    mElinkMerger{}
{
  impl::assertIsInRange("linkId", linkId, 0, 23);
}

template <typename FORMAT, typename CHARGESUM>
void GBTEncoder<FORMAT, CHARGESUM>::addChannelData(uint8_t elinkId, uint8_t chId,
                                                   const std::vector<SampaCluster>& data)
{
  impl::assertIsInRange("elinkId", elinkId, 0, 39);
  mElinks[elinkId].addChannelData(chId, data);
}

template <typename FORMAT, typename CHARGESUM>
size_t GBTEncoder<FORMAT, CHARGESUM>::moveToBuffer(std::vector<uint8_t>& buffer)
{
  mElinkMerger(mGbtId, gsl::span<ElinkEncoder<FORMAT, CHARGESUM>>(begin(mElinks), end(mElinks)), mGbtWords);
  for (auto& elink : mElinks) {
    elink.clear();
  }
  return impl::moveBuffer(mGbtWords, buffer);
}

} // namespace o2::mch::raw

#endif
