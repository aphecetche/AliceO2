#include "sml.hpp"
#include <iostream>

namespace sml = boost::sml;

struct Event {
};
struct WaitingSync {
};

int main()
{
  auto event = sml::event<Event>;
  auto waitingSync = sml::state<WaitingSync>;
  WaitingSync a;
  return 0;
}
