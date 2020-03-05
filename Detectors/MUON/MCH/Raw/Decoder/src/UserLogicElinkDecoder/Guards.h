// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_USER_LOGIC_ELINK_DECODER_GUARDS_H
#define O2_MCH_RAW_USER_LOGIC_ELINK_DECODER_GUARDS_H

#include "MCHRawCommon/SampaHeader.h"
#include "Debug.h"

namespace o2::mch::raw
{
// Guards

struct isSync {
  template <class EVT, class FSM, class SourceState, class TargetState>
  bool operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    return (evt.data == sampaSyncWord);
#ifdef ULDEBUG
    debugHeader(fsm) << fmt::format("isSync {}\n", (evt.data == sampaSyncWord));
#endif
  }
};

struct moreWordsToRead {
  template <class EVT, class FSM, class SourceState, class TargetState>
  bool operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
#ifdef ULDEBUG
    debugHeader(fsm) << fmt::format("moreWordsToRead {} n10 {}\n", (fsm.nof10BitWords > 0), fsm.nof10BitWords);
#endif
    return fsm.nof10BitWords > 0;
  }
};

struct moreDataAvailable {
  template <class EVT, class FSM, class SourceState, class TargetState>
  bool operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    bool rv = (fsm.maskIndex < fsm.masks.size());
#ifdef ULDEBUG
    debugHeader(fsm) << fmt::format("moreDataAvailable {} maskIndex {}\n", rv, fsm.maskIndex);
#endif
    return rv;
  }
};

struct moreSampleToRead {
  template <class EVT, class FSM, class SourceState, class TargetState>
  bool operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    bool rv = (fsm.clusterSize > 0);
#ifdef ULDEBUG
    debugHeader(fsm) << fmt::format("moreSampleToRead {} clustersize {}\n", rv, fsm.clusterSize);
#endif
    return rv;
  }
};

struct headerIsComplete {
  template <class EVT, class FSM, class SourceState, class TargetState>
  bool operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    bool rv = (fsm.headerParts.size() == 5);
#ifdef ULDEBUG
    debugHeader(fsm) << fmt::format("headerIsComplete {}\n", rv);
#endif
    return rv;
  }
};
} // namespace o2::mch::raw

#endif
