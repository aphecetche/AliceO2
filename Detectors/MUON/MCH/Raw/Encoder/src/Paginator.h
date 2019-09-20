// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction

#ifndef O2_MCH_RAW_ENCODER_PAGINATOR_H
#define O2_MCH_RAW_ENCODER_PAGINATOR_H

#include <gsl/span>
#include <vector>
#include <cstdlib>

namespace o2
{
namespace mch
{
namespace raw
{

/// paginateBuffer converts an input buffer composed of
/// (payloadHeader,payload) blocks
/// to a buffer of (RDH,payload) blocks of fixed size (the pageSize).
/// A RDH (derived from the input PayloadHeader) is present at the
/// beginning of each page.
///
/// @param buffer the input buffer containing (payloadHeader,payload) blocks
/// @param paginatedBuffer the output buffer containing (RDH,payload) blocks
/// @param pageSize size (in bytes) each output (RDH+payload) block will be
/// @param paddingByte the byte that will be used to fill in short pages
/// (the ones where payload < RDH - pageSize)
///
/// @return the number of pages in the paginatedBuffer.
///
template <typename RDH>
size_t paginateBuffer(gsl::span<const uint8_t> buffer,
                      std::vector<uint8_t>& paginatedBuffer,
                      size_t pageSize = 8192,
                      uint8_t paddingByte = 0xFF);
} // namespace raw
} // namespace mch
} // namespace o2

#endif
