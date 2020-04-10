// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "../Payload/CruBufferCreator.h"
#include "MCHRawCommon/DataFormats.h"
#include "MCHRawEncoder/DataBlock.h"
#include <iostream>
#include "Paginator.h"
#include "MCHRawCommon/RDHManip.h"
#include "Headers/RAWDataHeader.h"
#include <fstream>
#include <gsl/span>

using namespace o2::mch::raw;
using V4 = o2::header::RAWDataHeaderV4;

namespace o2::header
{
extern std::ostream& operator<<(std::ostream&, const o2::header::RAWDataHeaderV4&);
}

void generateCxxFile(std::ostream& out, gsl::span<const std::byte> pages)
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

  out << fmt::format("extern std::array<const uint8_t,{}> REF_BUFFER_CRU_BARE_CHARGESUM;\n",
                     pages.size_bytes());

  out << R"(
template <>
gsl::span<const std::byte> REF_BUFFER_CRU<o2::mch::raw::BareFormat, o2::mch::raw::ChargeSumMode>()
{
  return gsl::span<const std::byte>(reinterpret_cast<const std::byte*>(&REF_BUFFER_CRU_BARE_CHARGESUM[0]), REF_BUFFER_CRU_BARE_CHARGESUM.size());
}
)";

  out << fmt::format("std::array<const uint8_t, {}> REF_BUFFER_CRU_BARE_CHARGESUM = {{", pages.size_bytes());

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

int main()
{
  auto buffer = test::CruBufferCreator<BareFormat, ChargeSumMode>::makeBuffer(1);

  // try to minimize the output size by choosing the right page size
  // for the buffer
  std::array<size_t, 7> pageSizes = {128, 256, 512, 1024, 2048, 4096, 8192};
  std::array<size_t, pageSizes.size()> bufferSizes;

  for (auto i = 0; i < pageSizes.size(); i++) {
    std::vector<std::byte> pages;
    paginateBuffer<V4>(buffer, pages, pageSizes[i], std::byte{0});
    bufferSizes[i] = pages.size();
  }

  auto smallest = std::distance(bufferSizes.begin(), std::min_element(bufferSizes.begin(), bufferSizes.end()));
  auto smallestPageSize = pageSizes[smallest];

  std::vector<std::byte> pages;
  paginateBuffer<V4>(buffer, pages, smallestPageSize, std::byte{0});

  // set the chargesum mask for each rdh
  o2::mch::raw::forEachRDH<V4>(pages,
                               [](V4& rdh, gsl::span<std::byte>::size_type offset) {
                                 rdhFeeId(rdh, rdhFeeId(rdh) | 0x100);
                               });

  generateCxxFile(std::cout, pages);

  return 0;
}
