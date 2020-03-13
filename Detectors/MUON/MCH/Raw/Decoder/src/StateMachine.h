// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "sml.hpp"
#include <iostream>
#include "smlDump.h"
#include "smlLog.h"
#include "DecoderState.h"
#include "MCHRawCommon/DataFormats.h"
#include <boost/core/demangle.hpp>
#include "Debug.h"

namespace sml = boost::sml;
using namespace sml;
using namespace o2::mch::raw;

namespace o2::mch::raw
{

/// Events

struct RecoverableError {
  RecoverableError(const std::string& msg = "") : errorMessage{msg} {}
  std::string errorMessage;
};

struct NewData {
  NewData(uint64_t d50 = 0) : data{d50} {}
  uint64_t data;
};

struct Never {
};

/// Actions

const auto show = [](DecoderState& ds) {
  std::cout << "show: dsid=" << asString(ds.dsId()) << "\n";
};

const auto completeHeader = [](DecoderState& ds) {
  ds.completeHeader();
};

const auto readHeader = [](DecoderState& ds) {
  auto a = ds.pop10();
  ds.addHeaderPart(a);
};

const auto reset = [](DecoderState& ds) {
  ds.reset();
};

const auto setData = [](DecoderState& ds, auto event) {
  ds.setData(event.data);
};

template <typename CHARGESUM>
struct readSample;

template <>
struct readSample<ChargeSumMode> {
  void operator()(DecoderState& ds)
  {
    ds.decrementClusterSize();
    ds.decrementClusterSize();
    auto b = ds.pop10();
    auto a = ds.pop10();
    ds.addChargeSum(b, a);
  }
};

template <>
struct readSample<SampleMode> {
  void operator()(DecoderState& ds)
  {
    ds.decrementClusterSize();
    ds.addSample(ds.pop10());
  }
};

template <typename CHARGESUM>
struct readSize;

template <>
struct readSize<SampleMode> {
  void operator()(DecoderState& ds)
  {
    auto value = ds.pop10();
    ds.setClusterSize(value);
  }
};

template <>
struct readSize<ChargeSumMode> {
  void operator()(DecoderState& ds)
  {
    auto value = 2 * ds.pop10();
    ds.setClusterSize(value);
  }
};

const auto readTime = [](DecoderState& ds) {
  auto value = ds.pop10();
  ds.setClusterTime(value);
};

struct testAction {
  template <typename TEvent, typename TSM, typename TDeps, typename TSubs>
  void operator()(const TEvent& event, TSM& sm, TDeps& deps, TSubs& subs)
  {
    std::cout << "event=" << boost::core::demangle(typeid(event).name()) << "\n";
    std::cout << "sm=" << boost::core::demangle(typeid(sm).name()) << "\n";
    std::cout << "deps=" << boost::core::demangle(typeid(deps).name()) << "\n";
    const auto& ds = sml::aux::get<o2::mch::raw::DecoderState&>(deps);
    std::cout << "ds=" << boost::core::demangle(typeid(ds).name()) << "\n";
    std::cout << "subs=" << boost::core::demangle(typeid(subs).name()) << "\n";
    if (ds.hasError()) {
      std::cout << "HAS ERROR\n";
    }
    exit(1);
  }
};

/// Guards

// struct isSync {
//   template <typename TEvent, typename TSM, typename TDeps, typename TSubs>
//   bool operator()(const TEvent& event, TSM& sm, TDeps& deps, TSubs& subs)
//   // template <typename TEvent>
//   // bool operator()(TEvent& event)
//   {
//     constexpr uint64_t sampaSyncWord{0x1555540f00113};
//     return event.data == sampaSyncWord;
//   }
// };
//

const auto isSync = [](auto event) {
  constexpr uint64_t sampaSyncWord{0x1555540f00113};
  return event.data == sampaSyncWord;
};

const auto moreWordsToRead = [](const DecoderState& ds) {
  return ds.moreWordsToRead();
};

const auto moreDataAvailable = [](const DecoderState& ds) {
  return ds.moreDataAvailable();
};

const auto moreSampleToRead = [](const DecoderState& ds) {
  return ds.moreSampleToRead();
};

const auto headerIsComplete = [](const DecoderState& ds) {
  return ds.headerIsComplete();
};

template <typename CHARGESUM>
struct StateMachine {

  template <typename T>
  struct Normal {

    auto operator()() const
    {
      auto WaitingSync = state<class WaitingSync>;
      auto WaitingHeader = state<class WaitingHeader>;
      auto WaitingSize = state<class WaitingSize>;
      auto WaitingTime = state<class WaitingTime>;
      auto WaitingSample = state<class WaitingSample>;
      return make_transition_table(
        // clang-format off
      * WaitingSync + event<NewData> [ isSync ] / reset = WaitingHeader,
      //-----------------------------------------------------------------------------------------
      WaitingHeader + event<NewData> [ isSync ] / reset = WaitingHeader,
      WaitingHeader + event<NewData> [ not isSync ] / setData = WaitingHeader,
      WaitingHeader [ moreDataAvailable and not headerIsComplete ] / readHeader =  WaitingHeader,
      WaitingHeader [ headerIsComplete ] / completeHeader = WaitingSize,
      //-----------------------------------------------------------------------------------------
      WaitingSize + event<NewData> [ moreWordsToRead and not moreDataAvailable ] / setData = WaitingSize,
      WaitingSize [ moreDataAvailable and moreWordsToRead ] / readSize<CHARGESUM>{} = WaitingTime,
      //-----------------------------------------------------------------------------------------
      WaitingTime + event<NewData> [ moreWordsToRead and not moreDataAvailable ] / setData = WaitingTime,
      WaitingTime [ moreSampleToRead and moreDataAvailable ] / readTime = WaitingSample,
      //-----------------------------------------------------------------------------------------
      WaitingSample + event<NewData> [ moreSampleToRead ] / setData = WaitingSample,
      WaitingSample [ not moreSampleToRead and not moreWordsToRead ] = WaitingHeader,
      WaitingSample [ moreDataAvailable and moreSampleToRead ] / readSample<CHARGESUM>{} =  WaitingSample,
      WaitingSample [ moreWordsToRead and not moreSampleToRead ] = WaitingSize
      //-----------------------------------------------------------------------------------------
      );
      // clang-format on
    }
  };

  auto operator()() const
  {
    return make_transition_table(
      // clang-format off
                * state<Normal<CHARGESUM>> + event<RecoverableError> = "Errored"_s,
                "Errored"_s + event<NewData> / testAction{} = state<Normal<CHARGESUM>>
      // clang-format on
    );
  }
};

} // namespace o2::mch::raw
