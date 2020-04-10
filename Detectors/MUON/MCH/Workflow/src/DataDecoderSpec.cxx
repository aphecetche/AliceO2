// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

///
/// \file    runFileReader.cxx
/// \author  Andrea Ferrero
///
/// \brief This is an executable that reads a data file from disk and sends the data to QC via DPL.
///
/// This is an executable that reads a data file from disk and sends the data to QC via the Data Processing Layer.
/// It can be used as a data source for QC development. For example, one can do:
/// \code{.sh}
/// o2-qc-run-file-reader --infile=some_data_file | o2-qc --config json://${QUALITYCONTROL_ROOT}/etc/your_config.json
/// \endcode
///

#include <random>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include "Framework/CallbackService.h"
#include "Framework/ConfigParamRegistry.h"
#include "Framework/ControlService.h"
#include "Framework/DataProcessorSpec.h"
#include "Framework/Lifetime.h"
#include "Framework/Output.h"
#include "Framework/Task.h"
#include "Framework/WorkflowSpec.h"

#include "DPLUtils/DPLRawParser.h"
#include "MCHBase/Digit.h"
#include "Headers/RAWDataHeader.h"
#include "MCHRawCommon/DataFormats.h"
#include "MCHRawDecoder/PageDecoder.h"
#include "MCHRawElecMap/Mapper.h"
#include "MCHMappingFactory/CreateSegmentation.h"
#include "MCHRawCommon/RDHManip.h"

#include "MCHWorkflow/DataDecoderSpec.h"
//#include "RawBufferDecoder.h"

// Dans ce code, on récupère un infut aui est un message avec le buffer, on fait tourner le code de base decodeBuffer qui est dans Handlers, et on renvoir un message de sortie (inspiré de FileReader de Andrea)



namespace o2::header
{
extern std::ostream& operator<<(std::ostream&, const o2::header::RAWDataHeaderV4&);
}



namespace o2
{
namespace mch
{
namespace raw
{

using namespace o2;
using namespace o2::framework;
using namespace o2::mch::mapping;
using RDHv4 = o2::header::RAWDataHeaderV4;


std::array<int, 64> refManu2ds_st345 = {
    63, 62, 61, 60, 59, 57, 56, 53, 51, 50, 47, 45, 44, 41, 38, 35,
    36, 33, 34, 37, 32, 39, 40, 42, 43, 46, 48, 49, 52, 54, 55, 58,
    7, 8, 5, 2, 6, 1, 3, 0, 4, 9, 10, 15, 17, 18, 22, 25,
    31, 30, 29, 28, 27, 26, 24, 23, 20, 21, 16, 19, 12, 14, 11, 13};

int manu2ds(int i){
  return refManu2ds_st345[i];
}


class DataDecoderTask
{
  void decodeBuffer(gsl::span<const std::byte> page, std::vector<o2::mch::Digit> &digits)
  {
    size_t ndigits{0};

    auto channelHandler = [&](DsElecId dsElecId, uint8_t channel, o2::mch::raw::SampaCluster sc) {
      auto s = asString(dsElecId);
      channel = manu2ds(int(channel));
      if(mPrint) {
        auto ch = fmt::format("{}-CH{}", s, channel);
        std::cout << ch << std::endl;
      }
      double digitadc(0);
      //for (auto d = 0; d < sc.nofSamples(); d++) {
      for (auto d = 0; d < sc.samples.size(); d++) {
        digitadc += sc.samples[d];
      }


      int deId;
      int dsIddet;
      if(auto opt = Elec2Det(dsElecId); opt.has_value()) {
        DsDetId dsDetId = opt.value();
        dsIddet = dsDetId.dsId();
        deId = dsDetId.deId();
      }

      int padId = -1;
      try {
        const Segmentation& segment = segmentation(deId);
        //Segmentation segment(deId);

        padId = segment.findPadByFEE(dsIddet, int(channel));
        if(mPrint)
          std::cout << "DS "<<(int)dsElecId.elinkId()<<"  CHIP "<<((int)channel)/32<<"  CH "<<((int)channel)%32<<"  ADC " << digitadc << "  DE# " << deId << "  DSid " << dsIddet << "  PadId " << padId << std::endl;
      } catch (const std::exception& e) { return; }


      int time = 0;

      digits.emplace_back( o2::mch::Digit(time, deId, padId, digitadc) );
      //o2::mch::Digit& mchdigit = digits.back();
      //mchdigit.setDetID(deId);
      //mchdigit.setPadID(padId);
      //mchdigit.setADC(digitadc);
      //mchdigit.setTimeStamp(time);

      if(mPrint) std::cout << "DIGIT STORED:\nADC " << digits.back().getADC() << " DE# " << digits.back().getDetID() << " PadId " << digits.back().getPadID() << " time "<< digits.back().getTimeStamp() << std::endl;
      ++ndigits;
    };


    const auto patchPage = [&](gsl::span<const std::byte> rdhBuffer) {
      auto rdhPtr = reinterpret_cast<o2::header::RAWDataHeaderV4*>(const_cast<std::byte*>(&rdhBuffer[0]));
      auto& rdh = *rdhPtr;
      nrdhs++;
      auto cruId = rdhCruId(rdh);
      rdhFeeId(rdh, cruId * 2 + rdhEndpoint(rdh));
      if (true) {
        std::cout << nrdhs << "--" << rdh << "\n";
      }
    };

    o2::mch::raw::PageDecoder decode = o2::mch::raw::createPageDecoder(page, channelHandler);
    patchPage(page);
    decode(page);
  }


public:
  //_________________________________________________________________________________________________
  void init(framework::InitContext& ic)
  {
    Elec2Det = createElec2DetMapper<ElectronicMapperGenerated>();
    fee2Solar = o2::mch::raw::createFeeLink2SolarMapper<ElectronicMapperGenerated>();
    nrdhs = 0;

    mPrint = ic.options().get<bool>("print");
  }

  //_________________________________________________________________________________________________
  void run(framework::ProcessingContext& pc)
  {
    std::vector<o2::mch::Digit> digits;

    // get the input buffer
    auto& inputs = pc.inputs();
    DPLRawParser parser(inputs, o2::framework::select("TF:MCH/RAWDATA"));

    for (auto it = parser.begin(), end = parser.end(); it != end; ++it) {
      // retrieving RDH v4
      auto const* rdh = it.get_if<o2::header::RAWDataHeaderV4>();
      // retrieving the raw pointer of the page
      auto const* raw = it.raw();
      // retrieving payload pointer of the page
      //auto const* payload = it.data();
      // size of payload
      size_t payloadSize = it.size();
      // offset of payload in the raw page
      //size_t offset = it.offset();

      if( payloadSize == 0 ) continue;


      //std::cout<<"\n\npayloadSize: "<<payloadSize<<std::endl;
      //std::cout<<"raw:     "<<(void*)raw<<std::endl;
      //std::cout<<"payload: "<<(void*)payload<<std::endl;

      gsl::span<const std::byte> buffer(reinterpret_cast<const std::byte*>(raw), sizeof(o2::header::RAWDataHeaderV4)+payloadSize);
      decodeBuffer(buffer, digits);
    }

    for (auto&& input : pc.inputs()) {
      //QcInfoLogger::GetInstance() << "run RawDataProcessor: input " << input.spec->binding << AliceO2::InfoLogger::InfoLogger::endm;

      if (input.spec->binding != "readout")
        continue;

      const auto* header = o2::header::get<header::DataHeader*>(input.header);
      if (!header)
        continue;

      //std::cout<<"payloadSize: "<<header->payloadSize<<std::endl;

      gsl::span<const std::byte> buffer((const std::byte*)input.payload, header->payloadSize);
      decodeBuffer(buffer, digits);
    }

    if(mPrint) {
      for (auto d : digits) {
        std::cout <<
            " DE# " << d.getDetID() <<
            " PadId " << d.getPadID() <<
            " ADC " << d.getADC() <<
            " time "<< d.getTimeStamp() <<
            std::endl;
      }
    }

    const size_t OUT_SIZE = sizeof(o2::mch::Digit) * digits.size();

    /// send the output buffer via DPL
    char* outbuffer = NULL;
    outbuffer = (char*)realloc(outbuffer, OUT_SIZE);
    memcpy(outbuffer, digits.data(), OUT_SIZE);

    // create the output message
    auto freefct = [](void* data, void*) { free(data); };
    pc.outputs().adoptChunk(Output{ "MCH", "DIGITS", 0 }, outbuffer, OUT_SIZE, freefct, nullptr);
  }

private:
  std::function<std::optional<DsDetId>(DsElecId)> Elec2Det;
  std::function<std::optional<uint16_t>(FeeLinkId id)> fee2Solar;
  size_t nrdhs{0};

  std::ifstream mInputFile{}; ///< input file
  bool mPrint = false;        ///< print digits
};

//_________________________________________________________________________________________________
o2::framework::DataProcessorSpec getDecodingSpec()
{
  return DataProcessorSpec{
    "DataDecoder",
    //Inputs{InputSpec{"readout", "ROUT", "RAWDATA", Lifetime::Timeframe}},
    o2::framework::select("TF:MCH/RAWDATA"),
    //o2::framework::select("readout:ROUT/RAWDATA"),
    Outputs{OutputSpec{"MCH", "DIGITS", 0, Lifetime::Timeframe}},
    AlgorithmSpec{adaptFromTask<DataDecoderTask>()},
    Options{{"print", VariantType::Bool, false, {"print digits"}}}
  };
}

} // end namespace raw
} // end namespace mch
} // end namespace o2
