#include <Framework/DataSamplingReadoutAdapter.h>
#include "Framework/runDataProcessing.h"
#include "Framework/Task.h"
#include <iostream>
#include "MCHRaw/Decoder.h"
#include <fmt/format.h>

using namespace o2;
using namespace o2::framework;

auto hp = [](uint8_t cruId, uint8_t linkId, uint8_t chip,
             uint8_t channel, o2::mch::raw::SampaCluster sc) {
  // std::cout << fmt::format("CHIP {:2d} CH {:2d} ", chip, channel);
  // std::cout << sc << "\n";
};

auto rh = [](const o2::mch::raw::RAWDataHeader& rdh) {
  // std::cout << rdh << "\n";
  return true;
};

class Decoder
{
 public:
  void init(InitContext& ic)
  {
    mDecoder = o2::mch::raw::createBareDecoder(rh, hp, true);
    std::cout << "Create BareDecoder\n";
  }
  void run(ProcessingContext& pc)
  {
    std::cout << "HERE!\n";
    int i{0};
    for (auto&& input : pc.inputs()) {
      auto header = o2::header::get<header::DataHeader*>(input.header);
      header->print();
      auto payload = input.payload;
      gsl::span<uint32_t> buffer{reinterpret_cast<uint32_t*>(const_cast<char*>(payload)), static_cast<long>(header->payloadSize)};
      mDecoder(buffer);
    }
  }

 private:
  o2::mch::raw::Decoder mDecoder;
};

WorkflowSpec defineDataProcessing(ConfigContext const& context)
{
  WorkflowSpec specs;

  specs.push_back(
    specifyExternalFairMQDeviceProxy(
      "readout-proxy",
      Outputs{{{"readout"}, {"ROUT", "RAWDATA"}}},
      "type=sub,method=connect,address=ipc:///tmp/readout-pipe-1,rateLogging=1",
      dataSamplingReadoutAdapter({{"readout"}, {"ROUT", "RAWDATA"}})));

  DataProcessorSpec consumer{
    "consumer",
    select("readout:ROUT/RAWDATA"),
    Outputs{},
    adaptFromTask<Decoder>()};

  specs.push_back(consumer);
  return specs;
}
