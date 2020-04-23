// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_ENCODER_COUNTER_H
#define O2_MCH_RAW_ENCODER_COUNTER_H

#include <gsl/span>
#include <cstdint>
#include <map>

namespace o2::mch::raw
{
// setPacketCounter updates the packetCounter field of the RDHs
// where the packetCounter, for a given link, is incremented
// by one for each new RDH this link has been seen (modulo 255 as the
// packetCounter is only 8 bits)
template <typename RDH>
void setPacketCounter(gsl::span<std::byte> buffer,
                      std::map<uint16_t, uint8_t>& initialCounters);

} // namespace o2::mch::raw

#endif
