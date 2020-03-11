#ifndef O2_MCH_RAW_DECODER_SML_LOGGER_H
#define O2_MCH_RAW_DECODER_SML_LOGGER_H

#include "sml.hpp"
#include <cstdio>

namespace sml = boost::sml;

struct my_logger {
  template <class T>
  auto name()
  {
    return sml::aux::get_type_name<T>();
  }

  template <class SM, class TEvent>
  void log_process_event(const TEvent&)
  {
    printf("[%s][process_event] %s\n", name<SM>(), name<TEvent>());
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

#endif
