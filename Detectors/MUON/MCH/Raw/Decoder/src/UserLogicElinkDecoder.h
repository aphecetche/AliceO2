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

template <typename CHARGESUM>
class UserLogicElinkDecoder
{
 public:
  using StateMachineType = msm::back::state_machine<ul::Decoder_<CHARGESUM>>;

  UserLogicElinkDecoder(DsElecId dsId, SampaChannelHandler sampaChannelHandler)
    : mFSM(dsId, sampaChannelHandler)
  {
  }

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
  StateMachineType mFSM;
};

} // namespace o2::mch::raw
#endif
