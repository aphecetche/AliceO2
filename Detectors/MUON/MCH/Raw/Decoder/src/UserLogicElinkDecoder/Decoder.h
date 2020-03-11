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
#include "MCHRawDecoder/SampaChannelHandler.h"

namespace msm = boost::msm;
namespace mpl = boost::mpl;
using namespace msm::front;
using namespace msm::front::euml; // for Not_ operator

// for debugging states
#include <boost/msm/back/tools.hpp>
#include <boost/msm/back/metafunctions.hpp>
#include <boost/mpl/for_each.hpp>

namespace o2::mch::raw::ul
{
template <typename CHARGESUM>
struct TransitionTable : mpl::vector<
                           // clang-format off
  //
  // Source State      Event        Destination State   Action                  Guard
  //---------------------------------------------------------------------------------------------------------------------------------------------
  Row< WaitingSync   , NewData          , WaitingHeader     , reset                 , isSync                                         >,
  //---------------------------------------------------------------------------------------------------------------------------------
  Row< WaitingHeader , NewData          , WaitingHeader     , reset                 , isSync                                         >,
  Row< WaitingHeader , NewData          , WaitingHeader     , setData               , Not_<isSync>                                   >,
  Row< WaitingHeader,  none             , WaitingHeader     , readHeader            , And_<moreDataAvailable,Not_<headerIsComplete>> >,
  Row< WaitingHeader,  none             , WaitingSize       , completeHeader        , headerIsComplete                               >,
  //---------------------------------------------------------------------------------------------------------------------------------
  Row< WaitingSize   , NewData          , WaitingSize       , setData               , And_<moreWordsToRead,Not_<moreDataAvailable>>  >,
  Row< WaitingSize   , none             , WaitingTime       , readSize<CHARGESUM>   , And_<moreDataAvailable,moreWordsToRead>        >,
  //---------------------------------------------------------------------------------------------------------------------------------
  Row< WaitingTime   , NewData          , WaitingTime       , setData               , And_<moreWordsToRead,Not_<moreDataAvailable>>  >,
  Row< WaitingTime   , none             , WaitingSample     , readTime              , And_<moreSampleToRead,moreDataAvailable>       >,
  //---------------------------------------------------------------------------------------------------------------------------------
  Row< WaitingSample , NewData          , WaitingSample     , setData               , moreSampleToRead                               >,
  Row< WaitingSample , none             , WaitingHeader     , none                  , Not_<moreSampleToRead>                         >,
  Row< WaitingSample , none             , WaitingSample     , readSample<CHARGESUM> , And_<moreDataAvailable,moreSampleToRead>       >,
  Row< WaitingSample , none             , WaitingSize       , none                  , And_<moreWordsToRead,Not_<moreSampleToRead>>   >,
  //---------------------------------------------------------------------------------------------------------------------------
  Row< Stopped       , NewData          , WaitingSync       , none                  , none                                           >,
  //---------------------------------------------------------------------------------------------------------------------------
  Row< AllOk         , RecoverableError , Stopped           , none                  , none                                           >,
  Row< Stopped       , NewData          , AllOk             , none                  , none                                           >
  //---------------------------------------------------------------------------------------------------------------------------------------------
                           // clang-format on
                           > {
};

//
// Decoder = happy decoding state machine, i.e. what happens
// when there's no decoding error
//
template <typename CHARGESUM>
class Decoder_ : public msm::front::state_machine_def<Decoder_<CHARGESUM>>
{

 public:
  using initial_state = mpl::vector<AllOk, WaitingSync>;
  using transition_table = TransitionTable<CHARGESUM>;

  Decoder_(DsElecId mDsId, SampaChannelHandler sampaChannelHandler);

  bool headerIsComplete() const;
  bool moreDataAvailable() const;
  bool moreSampleToRead() const;
  bool moreWordsToRead() const;
  uint16_t data10(uint64_t value, size_t index) const;
  uint16_t pop10();
  void addChargeSum(uint16_t b, uint16_t a);
  void addHeaderPart(uint16_t a);
  void addSample(uint16_t sample);
  void completeHeader();
  void decrementClusterSize();
  void reset();
  std::string setClusterSize(uint16_t value);
  void setClusterTime(uint16_t value);
  void setData(uint64_t data);

  // Replaces the default no-transition response.
  template <class FSM, class Event>
  void no_transition(Event const& e, FSM&, int state)
  {
    typedef typename boost::msm::back::recursive_get_transition_table<FSM>::type recursive_stt;
    typedef typename boost::msm::back::generate_state_set<recursive_stt>::type all_states;
    std::string stateName;
    boost::mpl::for_each<all_states, boost::msm::wrap<boost::mpl::placeholders::_1>>(boost::msm::back::get_state_name<recursive_stt>(stateName, state));
    std::cout << "(Decoder.h) no transition from state " << boost::core::demangle(stateName.c_str())
              << " on event " << boost::core::demangle(typeid(e).name()) << std::endl;
  }

#ifdef ULDEBUG
  DsElecId
    dsId() const;
#endif

  Decoder_()
  {
    instance++;
  }

 public:
  static int instance;

 private:
  // mMasks used to access groups of 10 bits in a 50 bits range
  static constexpr std::array<uint64_t, 5> mMasks = {0x3FF, 0xFFC00, 0x3FF00000, 0xFFC0000000, 0x3FF0000000000};
  DsElecId mDsId{0, 0, 0};
  uint16_t mNof10BitWords{0};
  uint16_t mClusterSize{0};
  uint16_t mClusterTime{0};
  uint64_t mData{0};
  size_t mMaskIndex{0};
  std::vector<uint16_t> mSamples;
  std::vector<uint16_t> mHeaderParts;
  SampaHeader mSampaHeader;
  SampaChannelHandler mSampaChannelHandler;
}; // namespace o2::mch::raw::ul

template <typename CHARGESUM>
int Decoder_<CHARGESUM>::instance{0};

} // namespace o2::mch::raw::ul

#include "Decoder.inl"

#endif
