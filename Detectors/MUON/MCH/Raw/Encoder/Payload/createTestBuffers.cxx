// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "BufferCreator.h"
#include "DetectorsRaw/RawFileWriter.h"
#include "DumpBuffer.h"
#include "Headers/RAWDataHeader.h"
#include "MCHRawCommon/DataFormats.h"
#include "MCHRawEncoderPayload/DataBlock.h"
#include "MCHRawEncoderPayload/PayloadPaginator.h"
#include <boost/program_options.hpp>
#include <fstream>
#include <gsl/span>
#include <iostream>

namespace po = boost::program_options;
using namespace o2::mch::raw;
using V4 = o2::header::RAWDataHeaderV4;

namespace o2::header
{
extern std::ostream& operator<<(std::ostream&, const o2::header::RAWDataHeaderV4&);
}

void generateCxxFile(std::ostream& out, gsl::span<const std::byte> pages, bool userLogic, bool gbt)
{
  out << R"(// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "RefBuffers.h"
#include <array>
#include "MCHRawCommon/DataFormats.h"

)";

  const std::string baseName = gbt ? "GBT" : "CRU";

  const std::string arrayName = userLogic ? fmt::format("REF_BUFFER_{}_USERLOGIC_CHARGESUM", baseName) : fmt::format("REF_BUFFER_{}_BARE_CHARGESUM", baseName);

  out
    << fmt::format("extern std::array<const uint8_t,{}> {};\n", pages.size_bytes(), arrayName);

  out << fmt::format("template <> gsl::span<const std::byte> REF_BUFFER_{}<o2::mch::raw::{}, o2::mch::raw::ChargeSumMode>()\n", baseName, userLogic ? "UserLogicFormat" : "BareFormat");

  out << "{\n";

  out << fmt::format("return gsl::span<const std::byte>(reinterpret_cast<const std::byte*>(&{}[0]), {}.size());\n",
                     arrayName,
                     arrayName);

  out << "\n}\n";

  out
    << fmt::format("std::array<const uint8_t, {}> {}= {{", pages.size_bytes(), arrayName);

  out << R"(
// clang-format off
)";

  int i{0};
  for (auto v : pages) {
    out << fmt::format("0x{:02X}", v);
    if (i != pages.size_bytes() - 1) {
      out << ", ";
    }
    if (++i % 12 == 0) {
      out << "\n";
    }
  }

  out << R"(
// clang-format on
};
)";
}

int main(int argc, char** argv)
{
  po::options_description generic("options");
  bool userLogic{false};
  bool debugOnly{false};
  bool gbt{false};
  po::variables_map vm;
  std::string outputFile{""};

  // clang-format off
  generic.add_options()
      ("help,h", "produce help message")
      ("userLogic,u",po::bool_switch(&userLogic),"user logic format")
      ("gbt,g",po::bool_switch(&gbt),"gbt buffer")
      ("debug,d",po::bool_switch(&debugOnly),"debug only")
      ("output,o",po::value<std::string>(&outputFile),"output file");

  // clang-format on

  po::options_description cmdline;
  cmdline.add(generic);

  po::store(po::command_line_parser(argc, argv).options(cmdline).run(), vm);

  if (vm.count("help")) {
    std::cout << generic << "\n";
    return 2;
  }

  try {
    po::notify(vm);
  } catch (boost::program_options::error& e) {
    std::cout << "Error: " << e.what() << "\n";
    exit(1);
  }

  po::notify(vm);

  uint32_t orbit = 12345;
  uint16_t bc = 678;
  std::vector<std::byte> buffer;
  if (userLogic) {
    if (gbt) {
      buffer = test::GBTBufferCreator<UserLogicFormat, ChargeSumMode>::makeBuffer(11);
    } else {
      buffer = test::CruBufferCreator<UserLogicFormat, ChargeSumMode>::makeBuffer(1, orbit, bc);
    }
  } else {
    if (gbt) {
      buffer = test::GBTBufferCreator<BareFormat, ChargeSumMode>::makeBuffer(11);
    } else {
      buffer = test::CruBufferCreator<BareFormat, ChargeSumMode>::makeBuffer(1, orbit, bc);
    }
  }

  if (debugOnly && !gbt) {
    auto solar2feelink = createSolar2FeeLinkMapper<ElectronicMapperGenerated>();
    forEachDataBlockRef(
      buffer, [&](const DataBlockRef& ref) {
        std::cout << ref << "\n";
        std::cout << solar2feelink(ref.block.header.solarId).value() << "\n";
        if (userLogic) {
          impl::dumpBuffer<UserLogicFormat>(ref.block.payload);
        } else {
          impl::dumpBuffer<BareFormat>(ref.block.payload);
        }
      });
  }

  std::vector<std::byte> pages;
  if (!gbt) {
    const o2::raw::HBFUtils& hbfutils = o2::raw::HBFUtils::Instance();
    o2::conf::ConfigurableParam::setValue<uint32_t>("HBFUtils", "orbitFirst", orbit);
    o2::conf::ConfigurableParam::setValue<uint16_t>("HBFUtils", "bcFirst", bc);
    pages = paginate(buffer, userLogic);
  } else {
    pages = buffer;
  }

  if (debugOnly) {
    if (userLogic) {
      impl::dumpBuffer<UserLogicFormat>(pages);
    } else {
      impl::dumpBuffer<BareFormat>(pages);
    }
    return 0;
  }

  if (outputFile.size()) {
    std::ofstream out(outputFile);
    generateCxxFile(out, pages, userLogic, gbt);
  } else {
    generateCxxFile(std::cout, pages, userLogic, gbt);
  }
  return 0;
}
