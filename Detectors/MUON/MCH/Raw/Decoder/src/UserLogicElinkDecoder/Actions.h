// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_USER_LOGIC_ELINK_DECODER_ACTIONS_H
#define O2_MCH_RAW_USER_LOGIC_ELINK_DECODER_ACTIONS_H

#include <iostream>
#include <fmt/format.h>
#include "MCHRawCommon/DataFormats.h"
#include "Events.h"

namespace o2::mch::raw::ul
{
// Actions

struct reset {
  template <class EVT, class FSM, class SourceState, class TargetState>
  void operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    fsm.reset();
  }
};

template <typename CHARGESUM>
struct readSize {
  template <class EVT, class FSM, class SourceState, class TargetState>
  void operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt);
};

template <>
struct readSize<SampleMode> {
  template <class EVT, class FSM, class SourceState, class TargetState>
  void operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    auto value = fsm.pop10();
    auto msg = fsm.setClusterSize(value);
    if (!msg.empty()) {
      fsm.process_event(RecoverableError(msg));
    }
  }
};

template <>
struct readSize<ChargeSumMode> {
  template <class EVT, class FSM, class SourceState, class TargetState>
  void operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    auto value = 2 * fsm.pop10();
    fsm.setClusterSize(value);
  }
};

struct readTime {
  template <class EVT, class FSM, class SourceState, class TargetState>
  void operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    auto value = fsm.pop10();
    fsm.setClusterTime(value);
  }
};

template <typename CHARGESUM>
struct readSample {
  template <class EVT, class FSM, class SourceState, class TargetState>
  void operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt);
};

template <>
struct readSample<ChargeSumMode> {
  template <class EVT, class FSM, class SourceState, class TargetState>
  void operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    fsm.decrementClusterSize();
    fsm.decrementClusterSize();
    auto b = fsm.pop10();
    auto a = fsm.pop10();
    fsm.addChargeSum(b, a);
  }
};

template <>
struct readSample<SampleMode> {
  template <class EVT, class FSM, class SourceState, class TargetState>
  void operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    fsm.decrementClusterSize();
    fsm.addSample(fsm.pop10());
  }
};

struct readHeader {
  template <class EVT, class FSM, class SourceState, class TargetState>
  void operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    auto a = fsm.pop10();
    fsm.addHeaderPart(a);
  }
};

struct completeHeader {
  template <class EVT, class FSM, class SourceState, class TargetState>
  void operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    fsm.completeHeader();
  }
};

struct setData {
  template <class EVT, class FSM, class SourceState, class TargetState>
  void operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    fsm.setData(evt.data);
  }
};
} // namespace o2::mch::raw::ul
#endif
