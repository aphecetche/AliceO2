// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_ENCODER_RDH_PACKET_COUNTER_SETTER_H
#define O2_MCH_RAW_ENCODER_RDH_PACKET_COUNTER_SETTER_H

#include <gsl/span>
#include <cstdint>
#include <map>

namespace o2::mch::raw
{
// setPacketCounter updates the packetCounter field of the RDHs
// where the packetCounter, for a given feeId, is incremented
// by one for each new RDH this feeId has been seen (modulo 255 as the
// packetCounter is only 8 bits).
//
// the counts maps stores the value of the counts for each feeId,
// It is used on input to get the initial value per feeId.
// It is used on output to get the current value (after the
// function has performed its jobs) per feeId.
//
template <typename RDH>
void setPacketCounter(gsl::span<uint8_t> buffer,
                      std::map<uint16_t, uint8_t>& counts);

} // namespace o2::mch::raw

#endif
