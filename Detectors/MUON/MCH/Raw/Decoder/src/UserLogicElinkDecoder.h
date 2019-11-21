// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_USER_LOGIC_ELINK_DECODER_H
#define O2_MCH_RAW_USER_LOGIC_ELINK_DECODER_H

#include "MCHRawDecoder/Decoder.h"
#include <memory>

namespace o2::mch::raw
{
class UserLogicElinkDecoder
{
 public:
  UserLogicElinkDecoder(uint8_t cruId, uint8_t linkId, SampaChannelHandler sampaChannelHandler, bool chargeSumMode = true);

  void append(uint64_t data);

 private:
  struct Impl;
  std::shared_ptr<Impl> mFSM; // FIXME: try to turn this into a unique_ptr
};

} // namespace o2::mch::raw
#endif
