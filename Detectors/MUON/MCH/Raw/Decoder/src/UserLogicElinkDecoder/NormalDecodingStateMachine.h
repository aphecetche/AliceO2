// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_USER_LOGIC_ELINK_DECODER_STATE_MACHINE_H
#define O2_MCH_RAW_USER_LOGIC_ELINK_DECODER_STATE_MACHINE_H

#include <cstdlib>
#include <string>
#include "States.h"
#include "Guards.h"
#include "Actions.h"
#include "Debug.h"
#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/msm/front/euml/euml.hpp>

namespace msm = boost::msm;
namespace mpl = boost::mpl;
using namespace msm::front;
using namespace msm::front::euml; // for Not_ operator

namespace o2::mch::raw
{
template <typename CHARGESUM>
struct NormalTransitionTable : mpl::vector<
                                 // clang-format off
  //
  // Start             Event     Next            Action                                           Guard
  //---------------------------------------------------------------------------------------------------------------------------------------------
  Row< WaitingSync   , NewData , WaitingHeader , ActionSequence_<mpl::vector<foundSync,reset> > , isSync                                         >,
  //---------------------------------------------------------------------------------------------------------------------------------------------
  Row< WaitingHeader , NewData , WaitingHeader , ActionSequence_<mpl::vector<foundSync,reset>>  , isSync                                         >,
  Row< WaitingHeader , NewData , WaitingHeader , setData                                        , Not_<isSync>                                   >,
  Row< WaitingHeader,  none    , WaitingHeader , readHeader                                     , And_<moreDataAvailable,Not_<headerIsComplete>> >,
  Row< WaitingHeader,  none    , WaitingSize   , completeHeader                                 , headerIsComplete                               >,
  //---------------------------------------------------------------------------------------------------------------------------------------------
  Row< WaitingSize   , NewData , WaitingSize   , setData                                        , And_<moreWordsToRead,Not_<moreDataAvailable>>  >,
  Row< WaitingSize   , none    , WaitingTime   , readSize<CHARGESUM>                            , And_<moreDataAvailable,moreWordsToRead>        >,
  //---------------------------------------------------------------------------------------------------------------------------------------------
  Row< WaitingTime   , NewData , WaitingTime   , setData                                        , And_<moreWordsToRead,Not_<moreDataAvailable>>  >,
  Row< WaitingTime   , none    , WaitingSample , readTime                                       , And_<moreSampleToRead,moreDataAvailable>       >,
  //---------------------------------------------------------------------------------------------------------------------------------------------
  Row< WaitingSample , NewData , WaitingSample , setData                                        , moreSampleToRead                               >,
  Row< WaitingSample , none    , WaitingHeader , none                                           , Not_<moreSampleToRead>                         >,
  Row< WaitingSample , none    , WaitingSample , readSample<CHARGESUM>                          , And_<moreDataAvailable,moreSampleToRead>       >,
  Row< WaitingSample , none    , WaitingSize   , none                                           , And_<moreWordsToRead,Not_<moreSampleToRead>>   >
  //---------------------------------------------------------------------------------------------------------------------------------------------
                                 // clang-format on
                                 > {
};

//
// NormalDecodingStateMachine = happy decoding state machine, i.e. what happens
// when there's no decoding error
//
template <typename CHARGESUM>
struct NormalDecodingStateMachine_ : public msm::front::state_machine_def<NormalDecodingStateMachine_<CHARGESUM>> {

  using initial_state = mpl::vector<WaitingSync>;

  using transition_table = NormalTransitionTable<CHARGESUM>;

  uint16_t data10(uint64_t value, size_t index) const
  {
    if (index < 0 || index >= masks.size()) {
      std::cout << "-- ULDEBUG -- " << fmt::format("index {} is out of range\n", index);
      return 0;
    }
    uint64_t m = masks[index];
    return static_cast<uint16_t>((value & m) >> (index * 10) & 0x3FF);
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
#ifdef ULDEBUG
      std::stringstream s;
      s << SampaCluster(clusterTime, samples);
      debugHeader(*this) << fmt::format(" calling channelHandler for {} ch {} = {}\n",
                                        asString(dsId), channelNumber64(sampaHeader), s.str());
#endif
      channelHandler(dsId, channelNumber64(sampaHeader), SampaCluster(clusterTime, samples));
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
                   channelNumber64(sampaHeader),
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

} // namespace o2::mch::raw
#endif
