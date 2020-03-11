#include "sml.hpp"
#include <string>
#include <iostream>

namespace sml = boost::sml;
template <class T>
auto name()
{
  return boost::sml::aux::get_type_name<T>();
}

template <class T>
void dump_transition() noexcept
{
  auto src_state = std::string{sml::aux::string<typename T::src_state>{}.c_str()};
  auto dst_state = std::string{sml::aux::string<typename T::dst_state>{}.c_str()};
  if (dst_state == "X") {
    dst_state = "[*]";
  }

  if (T::initial) {
    std::cout << "[*] --> " << src_state << std::endl;
  }

  std::cout << src_state << " --> " << dst_state;

  const auto has_event = !sml::aux::is_same<typename T::event, sml::anonymous>::value;
  const auto has_guard = !sml::aux::is_same<typename T::guard, sml::front::always>::value;
  const auto has_action = !sml::aux::is_same<typename T::action, sml::front::none>::value;

  if (has_event || has_guard || has_action) {
    std::cout << " :";
  }

  if (has_event) {
    std::cout << " " << name<typename T::event>();
  }

  if (has_guard) {
    std::cout << " [" << name<typename T::guard>() << "]";
  }

  if (has_action) {
    std::cout << " / " << name<typename T::action>();
  }

  std::cout << std::endl;
}

template <template <class...> class T, class... Ts>
void dump_transitions(const T<Ts...>&) noexcept
{
  (dump_transition<Ts>(), ...);
}

template <class SM>
void dump(const SM&) noexcept
{
  std::cout << "@startuml" << std::endl
            << std::endl;
  dump_transitions(typename SM::transitions{});
  std::cout << std::endl
            << "@enduml" << std::endl;
}
