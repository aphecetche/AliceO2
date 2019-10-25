// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction

#ifndef O2_MCH_RAW_ENCODER_H
#define O2_MCH_RAW_ENCODER_H

#include <gsl/span>
#include <vector>
#include <cstdlib>

namespace o2
{
namespace mch
{
namespace raw
{

/// paginateBuffer converts an input buffer composed of (RDH,payload) blocks
/// (where payload size can be anything <= 16320 bytes)  without any gap,
/// to a buffer where a RDH is present at the beginning of each page
/// so that each (RDH,payload) block is exactly pageSize (in bytes) long.
///
/// return the number of pages in the paginatedBuffer.
///
/// If a payload is too small to fill the page, then the relevant number of
/// padding byte is inserted so that (RDH,payload,paddingByte)
/// is exactly pageSize.
///
size_t paginateBuffer(gsl::span<uint8_t> compactBuffer,
                      std::vector<uint8_t>& paginatedBuffer,
                      size_t pageSize = 8192,
                      uint8_t paddingByte = 0xFF);
} // namespace raw
} // namespace mch
} // namespace o2

#endif
