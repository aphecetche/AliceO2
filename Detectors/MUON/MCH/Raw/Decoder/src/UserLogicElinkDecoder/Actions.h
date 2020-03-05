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
#include "MCHRawCommon/SampaHeader.h"
#include "Events.h"
#include "Debug.h"

namespace o2::mch::raw
{
// Actions

struct reset {
  template <class EVT, class FSM, class SourceState, class TargetState>
  void operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    fsm.maskIndex = fsm.masks.size();
    fsm.headerParts.clear();
    fsm.clusterSize = 0;
    fsm.nof10BitWords = 0;
  }
};

struct reportError {
  template <class EVT, class FSM, class SourceState, class TargetState>
  void operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    static int n{0};
    std::cout << fmt::format("ERROR {:4d} {}\n", n, evt.message);
  }
};

struct foundSync {
  template <class EVT, class FSM, class SourceState, class TargetState>
  void operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    fsm.nofSync++;
  }
};

template <typename CHARGESUM>
struct readSize {
  template <class EVT, class FSM, class SourceState, class TargetState>
  void operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt);
};

// template <typename FSM>
// void checkSize(FSM& fsm)
// {
//   if (fsm.clusterSize == 0) {
//     fsm.process_event(Error("ClusterSize is zero"));
//   }
//   if (fsm.clusterSize + 2 > fsm.sampaHeader.nof10BitWords()) {
//     fsm.process_event(Error("ClusterSize is too big"));
//   }
// }
//
template <>
struct readSize<SampleMode> {
  template <class EVT, class FSM, class SourceState, class TargetState>
  void operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    fsm.clusterSize = fsm.pop10();
#ifdef ULDEBUG
    debugHeader(fsm)
      << " -> size=" << fsm.clusterSize << " maskIndex=" << fsm.maskIndex << "\n";
#endif
    if (fsm.clusterSize == 0) {
      fsm.process_event(ErrorFound("ClusterSize is zero"));
    } else if (fsm.clusterSize + 2 > fsm.sampaHeader.nof10BitWords()) {
      fsm.process_event(ErrorFound("ClusterSize is too big"));
    }
  }
};

template <>
struct readSize<ChargeSumMode> {
  template <class EVT, class FSM, class SourceState, class TargetState>
  void operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    fsm.clusterSize = 2 * fsm.pop10();
#ifdef ULDEBUG
    debugHeader(fsm)
      << " -> size=" << fsm.clusterSize << " maskIndex=" << fsm.maskIndex << "\n";
#endif
  }
};

struct readTime {
  template <class EVT, class FSM, class SourceState, class TargetState>
  void operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    fsm.clusterTime = fsm.pop10();
#ifdef ULDEBUG
    debugHeader(fsm)
      << " -> time=" << fsm.clusterTime << " maskIndex=" << fsm.maskIndex << "\n";
#endif
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
    fsm.clusterSize -= 2;
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
    --fsm.clusterSize;
    fsm.addSample(fsm.pop10());
  }
};

struct readHeader {
  template <class EVT, class FSM, class SourceState, class TargetState>
  void operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    auto a = fsm.pop10();
    fsm.addHeaderPart(a);
#ifdef ULDEBUG
    debugHeader(fsm)
      << fmt::format(">>>>> readHeader {:08X}", a);
    for (auto h : fsm.headerParts) {
      std::cout << fmt::format("{:4d} ", h);
    }
    std::cout << "\n";
#endif
  }
};

struct completeHeader {
  template <class EVT, class FSM, class SourceState, class TargetState>
  void operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    uint64_t header{0};
    for (auto i = 0; i < fsm.headerParts.size(); i++) {
      header += (static_cast<uint64_t>(fsm.headerParts[i]) << (10 * i));
    }

    fsm.sampaHeader = SampaHeader(header);
    fsm.nof10BitWords = fsm.sampaHeader.nof10BitWords();

#ifdef ULDEBUG
    debugHeader(fsm)
      << fmt::format(">>>>> completeHeader {:013X}\n", header)
      << "\n"
      << fsm.sampaHeader << "\n";
#endif

    fsm.headerParts.clear();
  }
};

struct setData {
  template <class EVT, class FSM, class SourceState, class TargetState>
  void operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    fsm.data = evt.data;
    fsm.maskIndex = 0;
#ifdef ULDEBUG
    debugHeader(fsm)
      << fmt::format(">>>>> setData {:08X} maskIndex {} 10bits=", fsm.data, fsm.maskIndex);
    for (int i = 0; i < fsm.masks.size(); i++) {
      std::cout << fmt::format("{:2d} ", fsm.data10(fsm.data, i));
    }
    std::cout << "\n";
#endif
  }
};
} // namespace o2::mch::raw
#endif
