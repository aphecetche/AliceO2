// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "BareElinkDecoder.h"

/// complete specializations of some methods

namespace o2::mch::raw
{
template <>
void BareElinkDecoder<ChargeSumMode>::sendCluster()
{
  if (mSampaChannelHandler) {
    mSampaChannelHandler(mCruId,
                         mLinkId,
                         mSampaHeader.chipAddress(),
                         mSampaHeader.channelAddress(),
                         SampaCluster(mTimestamp, mClusterSum));
  }
}

template <>
void BareElinkDecoder<SampleMode>::sendCluster()
{
  if (mSampaChannelHandler) {
    mSampaChannelHandler(mCruId,
                         mLinkId,
                         mSampaHeader.chipAddress(),
                         mSampaHeader.channelAddress(),
                         SampaCluster(mTimestamp, mSamples));
  }
  mSamples.clear();
}

template <>
void BareElinkDecoder<ChargeSumMode>::changeToReadingData()
{
  changeState(State::ReadingClusterSum, 20);
}

template <>
void BareElinkDecoder<SampleMode>::changeToReadingData()
{
  changeState(State::ReadingSample, 10);
}

} // namespace o2::mch::raw
