// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "UserLogicDSDecoder.h"

#include "MCHRawCommon/SampaHeader.h"
#include <algorithm>
#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/euml/euml.hpp>
#include <boost/msm/front/functor_row.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <fmt/format.h>

using namespace std;
namespace msm = boost::msm;
namespace mpl = boost::mpl;
using namespace msm::front;
using namespace msm::front::euml; // for Not_ operator
namespace o2::mch::raw
{

struct NewData {
  NewData(uint64_t d) : data{d} {}
  uint64_t data;
};

struct SyncNotFound {
};

// States

struct NamedState : public msm::front::state<> {
  NamedState(const char* name_) : name{name_} {}
  // template <class Event, class FSM>
  // void on_entry(const Event&, const FSM&)
  // {
  //   std::cout << "entering " << name << "\n";
  // }
  // template <class Event, class FSM>
  // void on_exit(const Event&, const FSM&)
  // {
  //   std::cout << "leaving " << name << "\n";
  // }
  std::string name;
};

struct WaitingSync : public NamedState {
  WaitingSync() : NamedState("WaitingSync") {}
};

struct WaitingHeader : public NamedState {
  WaitingHeader() : NamedState("WaitingHeader") {}
};

struct ReadingSize : public NamedState {
  ReadingSize() : NamedState("ReadingSize") {}
};

struct ReadingTime : public NamedState {
  ReadingTime() : NamedState("ReadingTime") {}
};

struct ReadingSample : public NamedState {
  ReadingSample() : NamedState("ReadingSample") {}
};

struct WaitingData : public NamedState {
  WaitingData() : NamedState("WaitingData") {}
};

struct saveData {
  template <class EVT, class FSM, class SourceState, class TargetState>
  void operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    fsm.data = evt.data;
  }
};

struct resetIndex {
  template <class EVT, class FSM, class SourceState, class TargetState>
  void operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    fsm.maskIndex = 0;
  }
};

// Guards
struct atLeastOneSync {
  template <class EVT, class FSM, class SourceState, class TargetState>
  bool operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    return fsm.nofSync > 0;
  }
};

struct syncFound {
  template <class EVT, class FSM, class SourceState, class TargetState>
  bool operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    const uint64_t sync = sampaSync().uint64();
    if (evt.data == sync) {
      fsm.nofSync++;
      return true;
    }
    return false;
  }
};

struct validHeader {
  template <class EVT, class FSM, class SourceState, class TargetState>
  bool operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    fsm.sampaHeader = SampaHeader(evt.data);
    fsm.nof10BitWords = fsm.sampaHeader.nof10BitWords();
    return true;
  }
};

struct validSize {
  template <class EVT, class FSM, class SourceState, class TargetState>
  bool operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    auto n = fsm.data10(evt.data, fsm.maskIndex);
    return n > 0 && n < 1024;
  }
};

struct moreWordsToRead {
  template <class EVT, class FSM, class SourceState, class TargetState>
  bool operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    return fsm.nof10BitWords > 0;
  }
};

struct moreDataToRead {
  template <class EVT, class FSM, class SourceState, class TargetState>
  bool operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    return (fsm.maskIndex < fsm.masks.size());
  }
};

// Actions

struct readSize {
  template <class EVT, class FSM, class SourceState, class TargetState>
  void operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    fsm.clusterSize = fsm.pop10();
  }
};

struct readTime {
  template <class EVT, class FSM, class SourceState, class TargetState>
  void operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    fsm.clusterTime = fsm.pop10();
  }
};

struct readSample {
  template <class EVT, class FSM, class SourceState, class TargetState>
  void operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    fsm.addSample(fsm.pop10());
  }
};

struct StateMachine_ : public msm::front::state_machine_def<StateMachine_> {

  typedef WaitingSync initial_state;

  struct transition_table : mpl::vector<
                              // clang-format off
  //   Start            Event         Next               Action            Guard
  // +---------------+---------+---------------+-----------------------+-------------+
  Row< WaitingSync   , NewData , WaitingHeader , none                  , syncFound               >,
  Row< WaitingSync   , none    , WaitingHeader , none                  , atLeastOneSync          >,
  Row< WaitingHeader , NewData , ReadingSize   , resetIndex            , validHeader             >,
  Row< ReadingSize   , NewData , ReadingTime   , ActionSequence_
                                                 <mpl::vector<
                                                 saveData,
                                                 readSize>>            , validSize               >,
  Row< ReadingTime   , none    , ReadingSample , readTime              , none                    >,
  Row< ReadingSample , none    , ReadingSample , readSample            , And_<moreDataToRead,
                                                                              moreWordsToRead>   >,
  Row< ReadingSample , none    , WaitingData   , none                  , Not_<
                                                                           And_<moreDataToRead,
                                                                                moreWordsToRead>>>,
  Row< WaitingData   , NewData , ReadingSample , ActionSequence_
                                                 <mpl::vector<
                                                 saveData,resetIndex>> , moreWordsToRead         >,
  Row< WaitingData   , none    , WaitingSync   , none                  , Not_<moreWordsToRead>   >
                              // clang-format on
                              > {
  };
  uint16_t data10(uint64_t value, size_t index) const
  {
    if (index < 0 || index >= masks.size()) {
      std::cout << fmt::format("index {} is out of range\n", index);
      return 0;
    }
    uint64_t m = masks[index];
    return static_cast<uint16_t>(((value & m) >> (masks.size() - 1 - index) * 10) & 0x3FF);
  }
  uint16_t pop10()
  {
    auto rv = data10(data, maskIndex);
    nof10BitWords = std::max(0, nof10BitWords - 1);
    maskIndex = std::min(masks.size(), maskIndex + 1);
    return rv;
  }
  void addSample(uint16_t sample)
  {
    samples.emplace_back(sample);
    if (samples.size() == clusterSize) {
      // a cluster is ready, send it
      channelHandler(cruId,
                     linkId,
                     sampaHeader.chipAddress(),
                     sampaHeader.channelAddress(),
                     SampaCluster(clusterTime, samples));
      samples.clear();
    }
  }
  // masks used to access groups of 10 bits in a 50 bits range
  std::array<uint64_t, 5> masks = {0x3FF0000000000, 0xFFC0000000, 0x3FF00000, 0xFFC00, 0x3FF};
  uint8_t cruId;
  uint8_t linkId;
  uint16_t nof10BitWords{0};
  uint16_t clusterSize{0};
  uint16_t clusterTime{0};
  uint64_t data{0};
  size_t maskIndex{0};
  size_t nofSync{0};
  std::vector<uint16_t> samples;
  SampaHeader sampaHeader;
  SampaChannelHandler channelHandler;
};

struct UserLogicDSDecoder::Impl : public msm::back::state_machine<StateMachine_> {
};

UserLogicDSDecoder::UserLogicDSDecoder(uint8_t cruId, uint8_t linkId, SampaChannelHandler sampaChannelHandler, bool chargeSumMode) : mFSM(new UserLogicDSDecoder::Impl)
{
  mFSM->cruId = cruId;
  mFSM->linkId = linkId;
  mFSM->channelHandler = sampaChannelHandler;
}

void UserLogicDSDecoder::append(uint64_t data)
{
  mFSM->process_event(NewData(data));
}

} // namespace o2::mch::raw
