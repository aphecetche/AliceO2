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

template <typename T>
struct DataFormatSizeFactor;

template <>
struct DataFormatSizeFactor<SampleMode> {
  static constexpr uint8_t value = 1;
};

template <>
struct DataFormatSizeFactor<ChargeSumMode> {
  static constexpr uint8_t value = 2;
};

/// Events

struct RecoverableError {
  RecoverableError(const std::string& msg = "") : errorMessage{msg} {}
  std::string errorMessage;
};

struct NewData {
  NewData(uint64_t d50 = 0) : data{d50} {}
  uint64_t data;
};

template <typename CHARGESUM>
struct StateMachine {

  struct Normal {

    explicit Normal(DecoderState& decoderState) : ds{decoderState}
    {
      std::cout << "Normal ds=" << ds << "\n";
    }
    DecoderState& ds;

    auto operator()() const
    {
      auto WaitingSync = state<class WaitingSync>;
      auto WaitingHeader = state<class WaitingHeader>;
      auto WaitingSize = state<class WaitingSize>;
      auto WaitingTime = state<class WaitingTime>;
      auto WaitingSample = state<class WaitingSample>;

      // guards

      const auto isSync = [this](auto event) {
        std::cout << "isSync ds=" << this->ds << "\n";
        constexpr uint64_t sampaSyncWord{0x1555540f00113};
        return event.data == sampaSyncWord;
      };

      const auto moreWordsToRead = [this]() {
        std::cout << "guard: moreWordsToRead\n";
        std::cout << ds << "\n";
        return ds.moreWordsToRead();
      };

      const auto moreDataAvailable = [this]() {
        return ds.moreDataAvailable();
      };

      const auto moreSampleToRead = [this]() {
        return ds.moreSampleToRead();
      };

      const auto headerIsComplete = [this]() {
        return ds.headerIsComplete();
      };

      // actions

      const auto readHeader = [this]() {
        auto a = ds.pop10();
        ds.addHeaderPart(a);
      };

      const auto reset = [this]() {
        ds.reset();
      };

      const auto setData = [this](auto event) {
        std::cout << "action: setData\n";
        std::cout << ds << "\n";
        ds.setData(event.data);
      };

      const auto readTime = [this]() {
        auto value = ds.pop10();
        ds.setClusterTime(value);
      };

      const auto completeHeader = [this]() {
        ds.completeHeader();
      };

      const auto readSample = [this]() {
        ds.decrementClusterSize();
        ds.addSample<CHARGESUM>(ds.pop10());
      };

      const auto readSize = [this]() {
        auto factor = DataFormatSizeFactor<CHARGESUM>::value;
        auto value = factor * ds.pop10();
        ds.setClusterSize(value);
      };

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
      WaitingSize [ moreDataAvailable and moreWordsToRead ] / readSize = WaitingTime,
      //-----------------------------------------------------------------------------------------
      WaitingTime + event<NewData> [ moreWordsToRead and not moreDataAvailable ] / setData = WaitingTime,
      WaitingTime [ moreSampleToRead and moreDataAvailable ] / readTime = WaitingSample,
      //-----------------------------------------------------------------------------------------
      WaitingSample + event<NewData> [ moreSampleToRead ] / setData = WaitingSample,
      WaitingSample [ not moreSampleToRead and not moreWordsToRead ] = WaitingHeader,
      WaitingSample [ moreDataAvailable and moreSampleToRead ] / readSample =  WaitingSample,
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
                * state<Normal> + event<RecoverableError> = "Errored"_s,
                "Errored"_s + event<NewData> = state<Normal>
      // clang-format on
    );
  }
};

} // namespace o2::mch::raw
