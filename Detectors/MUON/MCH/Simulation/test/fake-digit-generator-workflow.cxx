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

#include "Framework/WorkflowSpec.h"
#include "Framework/DataProcessorSpec.h"
#include "DPLUtils/MakeRootTreeWriterSpec.h"
#include "Framework/InputSpec.h"
#include "MCHBase/Digit.h"
#include <vector>
#include "Framework/runDataProcessing.h"
#include "DataFormatsMCH/ROFRecord.h"
#include <iostream>

using namespace o2::framework;

template <typename T>
using BranchDefinition = MakeRootTreeWriterSpec::BranchDefinition<T>;

bool generate(std::vector<o2::mch::Digit>& digits,
              std::vector<o2::mch::ROFRecord>& irs)
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

WorkflowSpec defineDataProcessing(const ConfigContext& cc)
{

  return WorkflowSpec{
    DataProcessorSpec{
      "mch-fake-digit-generator",
      {},
      {OutputSpec{{"mchdigits"}, "MCH", "DIGITS"},
       OutputSpec{{"mchrofrecords"}, "MCH", "DIGITSROF"}},
      AlgorithmSpec{
        [](ProcessingContext& pc) {
          auto& out = pc.outputs();
          std::vector<o2::mch::Digit> digits;
          std::vector<o2::mch::ROFRecord> irs;
          bool end = generate(digits, irs);
          for (auto& r : irs) {
            std::cout << r << "\n";
          }
          out.snapshot(Output{"MCH", "DIGITS"}, digits);
          out.snapshot(Output{"MCH", "DIGITSROF"}, irs);
          if (end) {
            pc.services().get<ControlService>().endOfStream();
          }
        }}},
    getMCHDigitWriterSpec()};
}
