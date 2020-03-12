#include "UserLogicElinkDecoder.h"
#include "MCHRawCommon/DataFormats.h"
#include <iostream>
#include "Debug.h"

int main()
{
  const auto channelHandler = [](o2::mch::raw::DsElecId dsId,
                                 uint8_t channel,
                                 o2::mch::raw::SampaCluster) {
    std::cout << "channelHandler called !\n";
  };

  constexpr uint64_t sampaSyncWord{0x1555540f00113};

  o2::mch::raw::UserLogicElinkDecoder<SampleMode> ds{DsElecId{0, 0, 2}, channelHandler};

  ds.append(sampaSyncWord);
  ds.append(0x1722e9f00327d);
  ds.append(1);
  ds.append(2);

  ds.status();
  return 0;
}
