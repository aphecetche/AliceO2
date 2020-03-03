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

#include "MCHRawCommon/DataFormats.h"
#include "MCHRawCommon/SampaHeader.h"
#include "MCHRawDecoder/Decoder.h"
#include <algorithm>
#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/euml/euml.hpp>
#include <boost/msm/front/functor_row.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <fmt/format.h>
#include <memory>
#include "MCHRawElecMap/DsElecId.h"
#include <array>

namespace msm = boost::msm;
namespace mpl = boost::mpl;
using namespace msm::front;
using namespace msm::front::euml; // for Not_ operator

// for debugging purposes only
// uncomment this line to get a very verbose output
#define ULDEBUG

template <typename FSM>

std::ostream& debugHeader(FSM& fsm)
{
  std::cout << "--ULDEBUG--" << fsm.dsId << "--";
  return std::cout;
}

namespace o2::mch::raw
{

constexpr uint64_t FIFTYBITSATONE = 0x3FFFFFFFFFFFF;

struct NewData {
  NewData(uint64_t d) : data{d & FIFTYBITSATONE} {}
  uint64_t data;
};

struct Never {
};

// States

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

// Actions

struct foundSync {
  template <class EVT, class FSM, class SourceState, class TargetState>
  void operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
  {
    fsm.nofSync++;
    fsm.maskIndex = fsm.masks.size();
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
    fsm.clusterSize = fsm.pop10();
#ifdef ULDEBUG
    debugHeader(fsm)
      << " -> size=" << fsm.clusterSize << " maskIndex=" << fsm.maskIndex << "\n";
#endif
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
      uint64_t a = (static_cast<uint64_t>(fsm.headerParts[i]) << (10 * i));
      header += a;
      debugHeader(fsm)
        << fmt::format(">>>>> completeHeader {:2d} = {:013X} + {:013X}\n", i, a, header);
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

template <typename CHARGESUM>
struct StateMachine_ : public msm::front::state_machine_def<StateMachine_<CHARGESUM>> {

  typedef WaitingSync initial_state;

  struct transition_table : mpl::vector<
                              // clang-format off
  //   Start            Event         Next               Action            Guard
  Row< WaitingSync   , NewData , WaitingHeader , foundSync             , isSync                  >,

  Row< WaitingHeader , NewData , WaitingHeader , ActionSequence_<
                                                 mpl::vector<setData>> , Not_<isSync>            >,


  Row< WaitingHeader,  none    , WaitingHeader , readHeader            , And_<moreDataAvailable,
                                                                              Not_<headerIsComplete>> >,
  Row< WaitingHeader,  none    , WaitingSize   , completeHeader        , headerIsComplete>,


  Row< WaitingSize   , NewData , WaitingSize   , setData               , And_<moreWordsToRead,
                                                                           Not_<moreDataAvailable>> >,
  Row< WaitingSize   , none    , WaitingTime   , readSize<CHARGESUM>   , And_<moreDataAvailable,
                                                                              moreWordsToRead>       >,
  Row< WaitingTime   , NewData , WaitingTime   , setData               , And_<moreWordsToRead,
                                                                               Not_<moreDataAvailable>> >,
  Row< WaitingTime   , none    , WaitingSample , readTime              , And_<moreSampleToRead,
                                                                         moreDataAvailable>      >,

  Row< WaitingSample , NewData , WaitingSample , setData               , moreSampleToRead       >,
  Row< WaitingSample , none    , WaitingHeader , none                  , Not_<moreSampleToRead>       >,

  Row< WaitingSample , none    , WaitingSample , readSample<CHARGESUM> , And_<moreDataAvailable,
                                                                              moreSampleToRead>  >,
  Row< WaitingSample , none    , WaitingSize   , none                  , And_<moreWordsToRead,
                                                                         Not_<moreSampleToRead>>>
                              // clang-format on
                              > {
  };
  uint16_t data10(uint64_t value, size_t index) const
  {
    if (index < 0 || index >= masks.size()) {
      std::cout << "-- ULDEBUG -- " << fmt::format("index {} is out of range\n", index);
      return 0;
    }
    uint64_t m = masks[index];
    return static_cast<uint16_t>((value & m) >> (index * 10) & 0x3FF);
  }
  uint8_t channelNumber(const SampaHeader& sh)
  {
    return sh.channelAddress() + (sh.chipAddress() % 2) * 32;
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
#ifdef ULDEBUG
    debugHeader(*this)
      << "sample = " << sample << "\n";
#endif
    samples.emplace_back(sample);
    if (clusterSize == 0) {
      // a cluster is ready, send it
      channelHandler(dsId,
                     channelNumber(sampaHeader),
                     SampaCluster(clusterTime, samples));
      samples.clear();
    }
  }

  void addHeaderPart(uint16_t a)
  {
    headerParts.emplace_back(a);
  }

  void addChargeSum(uint16_t b, uint16_t a)
  {
    // a cluster is ready, send it
    uint32_t q = (((static_cast<uint32_t>(a) & 0x3FF) << 10) | (static_cast<uint32_t>(b) & 0x3FF));
#ifdef ULDEBUG
    debugHeader(*this)
      << "chargeSum = " << q << "\n";
#endif
    channelHandler(dsId,
                   channelNumber(sampaHeader),
                   SampaCluster(clusterTime, q));
  }

  // Replaces the default no-transition response.
  template <class FSM, class Event>
  void no_transition(Event const& e, FSM& fsm, int state)
  {
    debugHeader(*this)
      << "no transition from state " << state
      << " on event " << typeid(e).name() << std::endl;
  }

  // masks used to access groups of 10 bits in a 50 bits range
  const std::array<uint64_t, 5> masks = {0x3FF, 0xFFC00, 0x3FF00000, 0xFFC0000000, 0x3FF0000000000};
  // const std::array<uint64_t, 5> masks = {0x3FF0000000000, 0xFFC0000000, 0x3FF00000, 0xFFC00, 0x3FF};
  DsElecId dsId{0, 0, 0};
  uint16_t nof10BitWords{0};
  uint16_t clusterSize{0};
  uint16_t clusterTime{0};
  uint64_t data{0};
  size_t maskIndex{0};
  size_t nofSync{0};
  std::vector<uint16_t> samples;
  std::vector<uint16_t> headerParts;
  SampaHeader sampaHeader;
  SampaChannelHandler channelHandler;
};

template <typename CHARGESUM>
class UserLogicElinkDecoder
{
 public:
  UserLogicElinkDecoder(DsElecId dsId, SampaChannelHandler sampaChannelHandler)
    : mFSM{}
  {
    mFSM.dsId = dsId;
    mFSM.channelHandler = sampaChannelHandler;
    mFSM.headerParts.reserve(5);
  }

  void append(uint64_t data)
  {
    uint64_t data50 = data & FIFTYBITSATONE;
#ifdef ULDEBUG
    debugHeader(mFSM) << fmt::format("append({:016X})->50bits={:013X}\n", data, data50);
#endif
    mFSM.process_event(NewData(data50));
  }

 private:
  msm::back::state_machine<StateMachine_<CHARGESUM>> mFSM;
};

} // namespace o2::mch::raw
#endif
