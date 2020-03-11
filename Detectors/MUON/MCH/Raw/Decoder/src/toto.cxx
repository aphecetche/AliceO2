#include "UserLogicElinkDecoder.h"
#include "MCHRawCommon/DataFormats.h"
#include <iostream>

int main()
{
  const auto channelHandler = [](o2::mch::raw::DsElecId dsId,
                                 uint8_t channel,
                                 o2::mch::raw::SampaCluster) {
    std::cout << "channelHandler called !\n";
  };

  constexpr uint64_t sampaSyncWord{0x1555540f00113};

  o2::mch::raw::UserLogicElinkDecoder<SampleMode> ds1{DsElecId{0, 0, 1}, channelHandler};
  o2::mch::raw::UserLogicElinkDecoder<SampleMode> ds2{DsElecId{0, 0, 2}, channelHandler};

  ds1.append(12345);

  ds2.append(sampaSyncWord);
  ds2.append(0x1722e9f00327d);
  ds2.append(12234);
  ds1.status();
  ds2.status();
  // my_logger logger{};
  // sm<StateMachine, sml::logger<my_logger>> sm1{logger, ds1};
  // sm<StateMachine, sml::logger<my_logger>> sm2{logger, ds2};
  //
  // sm1.process_event(NewData(12345));
  // sm2.process_event(RecoverableError{});
  //
  // sm1.visit_current_states([](auto state) {
  //   std::cout << "sm1 state=" << state.c_str() << std::endl;
  // });
  // sm2.visit_current_states([](auto state) {
  //   std::cout << "sm2 state=" << state.c_str() << std::endl;
  // });
  return 0;
}
