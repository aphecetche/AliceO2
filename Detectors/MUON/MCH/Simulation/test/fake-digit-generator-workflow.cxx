// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

// a workflow  which creates fake MCH digits

#include "DPLUtils/MakeRootTreeWriterSpec.h"
#include "DataFormatsMCH/ROFRecord.h"
#include "Framework/DataProcessorSpec.h"
#include "Framework/InputSpec.h"
#include "Framework/Task.h"
#include "Framework/WorkflowSpec.h"
#include "Framework/runDataProcessing.h"
#include "MCHBase/Digit.h"
#include <iostream>
#include <vector>
#include <fmt/format.h>

using namespace o2::framework;

template <typename T>
using BranchDefinition = MakeRootTreeWriterSpec::BranchDefinition<T>;

enum ChannelIdKind : uint32_t {
  SolarDsCh = 5,
  DeDsCh = 6
};

struct ChannelId {
  union {
    uint32_t word;
    struct {
      uint32_t solar_kind : 3;
      uint32_t solar_mbz : 1;
      uint32_t solar_id : 12;
      uint32_t solar_ds : 11;
      uint32_t solar_ch : 5;
    };
    struct {
      uint32_t de_kind : 3;
      uint32_t de_mbz : 1;
      uint32_t de_id : 12;
      uint32_t de_ds : 11;
      uint32_t de_ch : 5;
    };
  };
};

std::ostream& operator<<(std::ostream& out, const ChannelId& c)
{
  if (c.solar_kind == SolarDsCh) {
    out << fmt::format("SOLAR {:4d} DS {:4d} CH {:2d}", c.solar_id, c.solar_ds, c.solar_ch);
  } else if (c.de_kind == DeDsCh) {
    out << fmt::format("DE {:4d} DS {:4d} CH {:2d}", c.de_id, c.de_ds, c.de_ch);
  }
  return out;
}

struct PedInfo {
  float mean;
  float sigma;
};

std::ostream& operator<<(std::ostream& out, const PedInfo& p)
{
  out << fmt::format("mean {:7.2f} sigma {:7.2f}", p.mean, p.sigma);
  return out;
}

bool generate(std::vector<o2::mch::Digit>& digits,
              std::vector<o2::mch::ROFRecord>& irs,
              const std::map<uint32_t, PedInfo>& pedinfo)
{
  static uint16_t n{0};

  o2::InteractionRecord ir{0, n};

  int padid = static_cast<int>(n);
  unsigned long adc = static_cast<unsigned long>(n);

  digits.emplace_back(o2::mch::Digit{100, padid, adc, {}, n});
  irs.emplace_back(o2::mch::ROFRecord{ir, static_cast<int>(n), 1});
  n++;
  return n > 10;
}

DataProcessorSpec getMCHDigitWriterSpec()
{
  return MakeRootTreeWriterSpec("MCHDigitWriter",
                                "mchdigits.root",
                                "o2sim",
                                -1, //default number of events
                                BranchDefinition<std::vector<o2::mch::Digit>>{InputSpec{"mchdigits", "MCH", "DIGITS"}, "MCHDigit"},
                                BranchDefinition<std::vector<o2::mch::ROFRecord>>{InputSpec{"mchrofrecords", "MCH", "DIGITSROF"}, "MCHROFRecords"})();
}
class PedGen
{
 public:
  void init(o2::framework::InitContext& ic)
  {
    auto input = ic.options().get<std::string>("input");
    std::ifstream in;
    in.exceptions(std::ifstream::failbit);
    try {
      in.open(input.c_str());
    } catch (const std::exception& e) {
      throw std::runtime_error(fmt::format("could not open file {}", input));
    }
    in.exceptions(std::ifstream::goodbit);
    std::string head;
    std::getline(in, head);
    int solar, ds, ch;
    float mean, sigma;
    std::string line;
    while (std::getline(in, line)) {
      std::replace(line.begin(), line.end(), ',', ' ');
      std::stringstream ss(line);
      while (ss >> solar >> ds >> ch >> mean >> sigma) {
        ChannelId c;
        c.solar_kind = ChannelIdKind::SolarDsCh;
        c.solar_id = solar;
        c.solar_ds = ds;
        c.solar_ch = ch;
        PedInfo ped{mean, sigma};
        mPedInfos[c.word] = ped;
      }
    }
  }
  void run(o2::framework::ProcessingContext& pc)
  {
    auto& out = pc.outputs();
    std::vector<o2::mch::Digit> digits;
    std::vector<o2::mch::ROFRecord> irs;
    bool end = generate(digits, irs, mPedInfos);
    for (auto& r : irs) {
      std::cout << r << "\n";
    }
    out.snapshot(Output{"MCH", "DIGITS"}, digits);
    out.snapshot(Output{"MCH", "DIGITSROF"}, irs);
    if (end) {
      pc.services().get<ControlService>().endOfStream();
    }
  }

 private:
  std::map<uint32_t, PedInfo> mPedInfos;
};

WorkflowSpec defineDataProcessing(const ConfigContext& cc)
{

  return WorkflowSpec{
    DataProcessorSpec{
      "mch-fake-digit-generator",
      {},
      {OutputSpec{{"mchdigits"}, "MCH", "DIGITS"},
       OutputSpec{{"mchrofrecords"}, "MCH", "DIGITSROF"}},
      AlgorithmSpec{o2::framework::adaptFromTask<PedGen>()},
      {ConfigParamSpec{"input", VariantType::String, "", {"csv file with pedestal mean and sigmas to generate"}}}},
    getMCHDigitWriterSpec()};
}
//workflowOptions.push_back(ConfigParamSpec{"selection-run", VariantType::Int, 2, {"selection type: 2 - run 2, 3 - run 3"}});
