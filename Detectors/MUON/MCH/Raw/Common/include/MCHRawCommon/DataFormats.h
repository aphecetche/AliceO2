// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_DATA_FORMATS_H
#define O2_MCH_RAW_DATA_FORMATS_H

#include <cstdint>

namespace o2::mch::raw
{

struct BareFormat {
};

struct UserLogicFormat {
};

struct ChargeSumMode {
  bool operator()() const { return true; }
};

struct SampleMode {
  bool operator()() const { return false; }
};

using uint10_t = uint16_t;
using uint20_t = uint32_t;
using uint50_t = uint64_t;

template <typename FORMAT>
struct isUserLogicFormat {
  static constexpr bool value = false;
};
template <typename FORMAT>
struct isBareFormat {
  static constexpr bool value = false;
};

template <>
struct isUserLogicFormat<UserLogicFormat> {
  static constexpr bool value = true;
};

template <>
struct isBareFormat<BareFormat> {
  static constexpr bool value = true;
};

template <typename CHARGESUM>
struct isChargeSumMode {
  static constexpr bool value = false;
};

template <typename CHARGESUM>
struct isSampleMode {
  static constexpr bool value = false;
};

template <>
struct isChargeSumMode<ChargeSumMode> {
  static constexpr bool value = true;
};

template <>
struct isSampleMode<SampleMode> {
  static constexpr bool value = true;
};
}; // namespace o2::mch::raw

#endif
