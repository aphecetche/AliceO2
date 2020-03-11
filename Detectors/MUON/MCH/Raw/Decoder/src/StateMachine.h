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

namespace sml = boost::sml;
using namespace sml;
using namespace o2::mch::raw;

namespace o2::mch::raw
{

/// Events

struct RecoverableError {
};

struct NewData {
  NewData(uint64_t d50) : data{d50} {}
  uint64_t data;
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

/// Guards

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
      WaitingTime [ moreSampleToRead and moreDataAvailable ] / readTime = WaitingSample
      //-----------------------------------------------------------------------------------------
//   Row< WaitingSample , NewData    , WaitingSample     , setData               , moreSampleToRead                               >,
//   Row< WaitingSample , none       , WaitingHeader     , none                  , Not_<moreSampleToRead>                         >,
//   Row< WaitingSample , none       , WaitingSample     , readSample<CHARGESUM> , And_<moreDataAvailable,moreSampleToRead>       >,
//   Row< WaitingSample , none       , WaitingSize       , none                  , And_<moreWordsToRead,Not_<moreSampleToRead>>   >
      );
      // clang-format on
    }
  };

  auto operator()() const
  {
    return make_transition_table(
      // clang-format off
                * state<Normal<CHARGESUM>> + event<RecoverableError> / show = "Errored"_s,
                "Errored"_s + event<NewData> = state<Normal<CHARGESUM>>
      // clang-format on
    );
  }
};

} // namespace o2::mch::raw
