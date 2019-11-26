// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_ENCODER_ELECTRONIC_MAPPER_IMPL_HELPER_H
#define O2_MCH_RAW_ENCODER_ELECTRONIC_MAPPER_IMPL_HELPER_H

#include <cstdint>

namespace o2::mch::raw::impl
{

inline uint32_t encodeDeDs(uint16_t a, uint16_t b)
{
  return a << 16 | b;
}

inline uint16_t decode_a(uint32_t x)
{
  return static_cast<uint16_t>((x & 0xFFFF0000) >> 16);
}
inline uint16_t decode_b(uint32_t x)
{
  return static_cast<uint16_t>(x & 0xFFFF);
}

inline uint16_t encodeSolarGroupIndex(uint16_t solarId, uint8_t groupId, uint8_t index)
{
  return (solarId & 0x3FF) | ((groupId & 0x7) << 10) |
         ((index & 0x7) << 13);
}

inline uint16_t decodeSolarId(uint16_t code)
{
  return code & 0x3FF;
}

inline uint8_t decodeGroupId(uint16_t code)
{
  return (code & 0x1C00) >> 10;
}

inline uint8_t decodeElinkIndex(uint16_t code)
{
  return (code & 0xE000) >> 13;
}
} // namespace o2::mch::raw::impl
#endif
