// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_USER_LOGIC_ELINK_DECODER_STATES_H
#define O2_MCH_RAW_USER_LOGIC_ELINK_DECODER_STATES_H

#include <boost/msm/front/state_machine_def.hpp>
#include <string>
#include "Debug.h"

namespace msm = boost::msm;
using namespace msm::front;

namespace o2::mch::raw
{
struct NamedState : public msm::front::state<> {
  NamedState(const char* name_) : name{name_} {}
#ifdef ULDEBUG
  template <class Event, class FSM>
  void on_entry(const Event&, const FSM& fsm)
  {
    debugHeader(fsm)
      << "--> " << name << "\n";
  }
  template <class Event, class FSM>
  void on_exit(const Event&, const FSM& fsm)
  {
    debugHeader(fsm)
      << name << "--> \n";
  }
#endif
  std::string name;
};

struct WaitingSync : public NamedState {
  WaitingSync() : NamedState("WaitingSync") {}
};

struct WaitingHeader : public NamedState {
  WaitingHeader() : NamedState("WaitingHeader") {}
};

struct WaitingSize : public NamedState {
  WaitingSize() : NamedState("WaitingSize") {}
};

struct WaitingTime : public NamedState {
  WaitingTime() : NamedState("WaitingTime") {}
};

struct WaitingSample : public NamedState {
  WaitingSample() : NamedState("WaitingSample") {}
};

struct AllOk : public NamedState {
  AllOk() : NamedState("AllOk") {}
};
// struct ErrorMode : public interrupt_state<Never> {
struct ErrorMode : public NamedState {
  ErrorMode() : NamedState("ErrorMode") {}
};

} // namespace o2::mch::raw

#endif
