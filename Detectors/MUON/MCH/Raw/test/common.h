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
#include <vector>
#include <array>
#include <gsl/span>

namespace o2
{
namespace mch
{
namespace raw
{
namespace test
{

int countRDHs(gsl::span<uint32_t> buffer);
int showRDHs(gsl::span<uint32_t> buffer);
void dumpBuffer(gsl::span<uint32_t> buffer);

o2::mch::raw::ElinkEncoder createElinkEncoder();

std::vector<uint32_t> createCRUBuffer(int cruId = 0);

std::vector<uint32_t> createGBTBuffer();

extern std::array<uint32_t, 640> REF_BUFFER;

} // namespace test
} // namespace raw
} // namespace mch
} // namespace o2

#endif
