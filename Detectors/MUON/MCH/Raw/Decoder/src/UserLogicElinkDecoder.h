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

#include "MCHRawDecoder/SampaChannelHandler.h"
#include "MCHRawElecMap/DsElecId.h"
#include "StateMachine.h"
#include "Debug.h"

#ifdef ULDEBUG
#include "StateMachineLogger.h"
#endif

namespace o2::mch::raw
{

template <typename CHARGESUM>
class UserLogicElinkDecoder
{
 public:
  UserLogicElinkDecoder(DsElecId dsId, SampaChannelHandler sampaChannelHandler)
    : mDecoderState{dsId, sampaChannelHandler},
#ifdef ULDEBUG
      mLogger{},
      mStateMachine
  {
    mLogger, mDecoderState
  }
#else
      mStateMachine
  {
    mDecoderState
  }
#endif
  {
  }

  void append(uint64_t data)
  {
    constexpr uint64_t FIFTYBITSATONE = 0x3FFFFFFFFFFFF;
    uint64_t data50 = data & FIFTYBITSATONE;
    mStateMachine.process_event(NewData(data50));
  }

  void status()
  {
    mStateMachine.visit_current_states([](auto state) {
      std::cout << "state=" << state.c_str() << std::endl;
    });
    std::cout << "decoder state=" << mDecoderState << "\n";
  }

 private:
  DecoderState mDecoderState;
#ifdef ULDEBUG
  Logger mLogger;
  boost::sml::sm<StateMachine<CHARGESUM>, boost::sml::logger<Logger>> mStateMachine;
#else
  boost::sml::sm<StateMachine<CHARGESUM>> mStateMachine;
#endif
};

} // namespace o2::mch::raw

#endif
