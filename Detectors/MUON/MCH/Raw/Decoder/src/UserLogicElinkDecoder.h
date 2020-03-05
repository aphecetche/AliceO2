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

// for debugging purposes only

#include "MCHRawCommon/DataFormats.h"
#include "MCHRawCommon/SampaHeader.h"
#include "MCHRawDecoder/Decoder.h"
#include "MCHRawElecMap/DsElecId.h"
#include "UserLogicElinkDecoder/Actions.h"
#include "UserLogicElinkDecoder/Guards.h"
#include "UserLogicElinkDecoder/NormalDecodingStateMachine.h"
#include "UserLogicElinkDecoder/States.h"
#include <algorithm>
#include <array>
#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/functor_row.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <fmt/format.h>
#include <memory>

namespace msm = boost::msm;
namespace mpl = boost::mpl;
using namespace msm::front;

namespace o2::mch::raw
{

template <typename CHARGESUM>
struct StateMachine_ : public msm::front::state_machine_def<StateMachine_<CHARGESUM>> {

  using Normal = msm::back::state_machine<NormalDecodingStateMachine_<CHARGESUM>>;

  using initial_state = Normal;

  struct transition_table : mpl::vector<
                              //  Start      Event       Next       Action   Guard
                              Row<Normal, ErrorFound, ErrorMode, none, none>,
                              Row<ErrorMode, NewData, Normal, none, none>> {
  };

  // StateMachine_(DsElecId _dsId, SampaChannelHandler channelHandler) : dsId{_dsId}
  // {
  //   //Normal(msm::back::states_ << WaitingSync(), _dsId, channelHandler);
  //   //player p( back::states_ << state_1 << ... << state_n , boost::ref(data),3);
  // }
  //DsElecId dsId;
};

template <typename CHARGESUM>
class UserLogicElinkDecoder
{
 public:
  UserLogicElinkDecoder(DsElecId dsId, SampaChannelHandler sampaChannelHandler)
  //  : mFSM(dsId, sampaChannelHandler)
  {
    mFSM.dsId = dsId;
    mFSM.channelHandler = sampaChannelHandler;
  }

  void append(uint64_t data)
  {
    uint64_t data50 = data & FIFTYBITSATONE;
    // #ifdef ULDEBUG
    //     debugHeader(mFSM) << fmt::format("append({:016X})->50bits={:013X}\n", data, data50);
    // #endif
    mFSM.process_event(NewData(data50));
  }

 private:
  // msm::back::state_machine<StateMachine_<CHARGESUM>> mFSM;
  msm::back::state_machine<NormalDecodingStateMachine_<CHARGESUM>> mFSM;
};

} // namespace o2::mch::raw
#endif
