// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_SAMPA_HEADER
#define O2_MCH_RAW_SAMPA_HEADER

#include <cstdlib>

namespace o2
{
namespace mch
{
namespace raw
{

class SampaHeader
{
 public:
  // value must but be 50 bits-wide max, and :
  // - bits 7-9 must be zero
  // - bits 24,26,28 must be one
  // - bits 25,27 must be zero
  // - bit 49 must be zero
  // otherwise the ctor throws
  // an exception
  explicit SampaHeader(uint64_t value);

  // hamming is 6 bits max
  // pkt is 3 bits max
  // numWords is 10 bits max
  // h is 4 bits max
  // ch is 5 bits max
  // bx is 20 bits max
  // if any of those limits are not respected, the ctor throws an exception
  explicit SampaHeader(uint8_t hamming,
                       bool p,
                       uint8_t pkt,
                       uint16_t numWords,
                       uint8_t h,
                       uint8_t ch,
                       uint32_t bx,
                       bool dp);

  bool operator==(const SampaHeader& rhs) const;
  bool operator!=(const SampaHeader& rhs) const;

  uint8_t hammingCode() const;
  bool headerParity() const;
  uint8_t packetType() const;
  uint16_t nbOf10BitWords() const;
  uint8_t chipAddress() const;
  uint8_t channelAddress() const;
  uint32_t bunchCrossingCounter() const;
  bool payloadParity() const;

  uint64_t asUint64() const { return mValue; }

 public:
  uint64_t mValue;
};

SampaHeader sampaSync();

} // namespace raw
} // namespace mch
} // namespace o2

#endif
