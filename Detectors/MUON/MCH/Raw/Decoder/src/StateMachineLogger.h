// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_DECODER_STATE_MACHINE_LOGGER_H
#define O2_MCH_RAW_DECODER_STATE_MACHINE_LOGGER_H

#include "sml.hpp"

namespace o2::mch::raw
{
struct Logger {
  template <class T>
  auto name()
  {
    return boost::sml::aux::get_type_name<T>();
  }

  template <class SM, class TEvent>
  void log_process_event(const TEvent&)
  {
    printf("[%s[process_event] %s\n", name<SM>(), name<TEvent>());
  }
  template <class SM, class TGuard, class TEvent>
  void log_guard(const TGuard&, const TEvent&, bool result)
  {
    printf("[%s][guard] %s %s %s\n", name<SM>(), name<TGuard>(), name<TEvent>(),
           (result ? "[OK]" : "[Reject]"));
  }
  template <class SM, class TAction, class TEvent>
  void log_action(const TAction&, const TEvent&)
  {
    printf("[%s][action] %s %s\n", name<SM>(), name<TAction>(), name<TEvent>());
  }
  template <class SM, class TSrcState, class TDstState>
  void log_state_change(const TSrcState& src, const TDstState& dst)
  {
    printf("[%s][transition] %s -> %s\n", name<SM>(), src.c_str(), dst.c_str());
  }
};
} // namespace o2::mch::raw

#endif
