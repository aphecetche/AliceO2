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
#include "UserLogicElinkDecoder/Decoder.h"
#include "UserLogicElinkDecoder/States.h"
#include "UserLogicElinkDecoder/Events.h"
#include <algorithm>
#include <array>
#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/functor_row.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <fmt/format.h>
#include <memory>

// for debugging states
#include <boost/msm/back/tools.hpp>
#include <boost/msm/back/metafunctions.hpp>
#include <boost/mpl/for_each.hpp>

namespace msm = boost::msm;
namespace mpl = boost::mpl;
using namespace msm::front;

namespace o2::mch::raw
{

struct ErrorMode : public ul::NamedState {
  ErrorMode() : ul::NamedState("ErrorMode") {}
};

struct ErrorFound {
  ErrorFound(const std::string& msg) : message{msg} {}
  std::string message;
};

static int nerr{0};

struct reportError {
  template <class EVT, class FSM, class SourceState, class TargetState>
  void operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    std::cout << fmt::format("Oh My ERROR {:4d} {}\n", nerr, evt.message);
    ++nerr;
    // std::cout << "this is the end\n";
    // exit(1);
  }
};

struct Never {
  template <class EVT, class FSM, class SourceState, class TargetState>
  bool operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    return false;
  }
};

template <typename CHARGESUM>
class StateMachine_ : public msm::front::state_machine_def<StateMachine_<CHARGESUM>>
{
 public:
  using Normal = msm::back::state_machine<ul::Decoder_<CHARGESUM>>;

  using initial_state = Normal;

  struct transition_table : mpl::vector<
                              Row<Normal, ErrorFound, ErrorMode, reportError, none>,
                              Row<ErrorMode, ul::NewData, Normal, none, none>> {
  };

  StateMachine_(DsElecId dsId) : mDsId{dsId}
  {
    instance++;
  }

  DsElecId dsId() const { return mDsId; }

  // Replaces the default no-transition response.
  template <class FSM, class Event>
  void no_transition(Event const& e, FSM&, int state)
  {
    typedef typename boost::msm::back::recursive_get_transition_table<FSM>::type recursive_stt;
    typedef typename boost::msm::back::generate_state_set<recursive_stt>::type all_states;
    std::string stateName;
    boost::mpl::for_each<all_states, boost::msm::wrap<boost::mpl::placeholders::_1>>(boost::msm::back::get_state_name<recursive_stt>(stateName, state));
    std::cout << "no transition from state " << boost::core::demangle(stateName.c_str())
              << " on event " << boost::core::demangle(typeid(e).name()) << std::endl;
  }

 private:
  DsElecId mDsId;

 public:
  static int instance;
}; // namespace o2::mch::raw

template <typename CHARGESUM>
int StateMachine_<CHARGESUM>::instance{0};

template <typename CHARGESUM>
class UserLogicElinkDecoder
{
 public:
  using StateMachineType = msm::back::state_machine<StateMachine_<CHARGESUM>>;

  UserLogicElinkDecoder(DsElecId dsId, SampaChannelHandler sampaChannelHandler)
    : mFSM{dsId}, mSampaChannelHandler{sampaChannelHandler}
  {
    init();
  }

  // UserLogicElinkDecoder(const UserLogicElinkDecoder& other)
  // {
  //   mFSM = other.mFSM;
  //   init();
  // }

  UserLogicElinkDecoder& operator=(const UserLogicElinkDecoder& other) = delete;

  void append(uint64_t data)
  {
    static int n{0};

    constexpr uint64_t FIFTYBITSATONE = 0x3FFFFFFFFFFFF;
    uint64_t data50 = data & FIFTYBITSATONE;
#ifdef ULDEBUG
    std::cout << fmt::format("{}--ULDEBUG--{:s}--(TOP)--{:4d}--append({:016X})->50bits={:013X} n={}\n", reinterpret_cast<const void*>(&mFSM), asString(mFSM.dsId()),
                             mFSM.instance, data, data50, n);
#endif
    mFSM.process_event(ul::NewData(data50));
    n++;
  }

 private:
  void init()
  {
    // The line below might require some explanation...
    //
    // First, you must realize that there are _two_ state machines
    // involved here. The first (mFSM) one is just the "steering" one,
    // handling the transition from normal mode to error mode.
    // The second one, the Normal state machine, is the real worker which
    // handles the decoding when everything feels right. It is a state
    // machine but also a state of its parent state machine (mFSM).
    //
    // So the line below calls the init() method of the Normal sub state machine
    // The init sets the dual sampa id, the sampa channel handler and
    // a special function that be will called (with a single string argument)
    // by the Normal sub state machine when there is an error.
    // In turn, _this_ (mFSM) state machine triggers an ErrorFound event,
    // which transition from the Normal state to the ErrorMode state.
    //
    mFSM.template get_state<typename StateMachine_<CHARGESUM>::Normal&>().init(mFSM.dsId(), mSampaChannelHandler,
                                                                               [this](std::string errorMessage) { this->mFSM.process_event(ErrorFound(errorMessage)); });
  }

 private:
  SampaChannelHandler mSampaChannelHandler;
  StateMachineType mFSM;
};

} // namespace o2::mch::raw
#endif
