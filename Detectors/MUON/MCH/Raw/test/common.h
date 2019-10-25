// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_TEST_COMMON_H
#define O2_MCH_RAW_TEST_COMMON_H

#include "MCHRaw/ElinkEncoder.h"
#include "MCHRaw/SampaChannelHandler.h"
#include <array>
#include <gsl/span>
#include <string>
#include <vector>

namespace o2
{
namespace mch
{
namespace raw
{
namespace test
{

SampaChannelHandler handlePacketPrint(std::string_view msg);

SampaChannelHandler handlePacketStoreAsVec(std::vector<std::string>& result);

o2::mch::raw::ElinkEncoder createElinkEncoder10();
o2::mch::raw::ElinkEncoder createElinkEncoder20();

std::vector<uint8_t> createCRUBuffer(int cruId = 0);

std::vector<uint8_t> createGBTBuffer();

extern std::array<uint8_t, 2560> REF_BUFFER;

} // namespace test
} // namespace raw
} // namespace mch
} // namespace o2

#endif
