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
#include <iostream>

namespace o2
{
namespace mch
{
namespace raw
{

enum class SampaPacketType : uint8_t {
  HeartBeat = 0,
  DataTruncated = 1,
  Sync = 2,
  DataTruncatedTriggerTooEarly = 3,
  Data = 4,
  DataNumWords = 5,
  DataTriggerTooEarly = 6,
  DataTriggerTooEarlyNumWords = 7
};

class SampaHeader
{
 public:
  explicit SampaHeader(uint64_t value = 0);

  // hamming is 6 bits max
  // pkt is 3 bits max
  // numWords is 10 bits max
  // h is 4 bits max
  // ch is 5 bits max
  // bx is 20 bits max
  // if any of those limits are not respected, the ctor throws an exception
  explicit SampaHeader(uint8_t hamming,
                       bool p,
                       SampaPacketType pkt,
                       uint16_t numWords,
                       uint8_t h,
                       uint8_t ch,
                       uint32_t bx,
                       bool dp);

  bool operator==(const SampaHeader& rhs) const;
  bool operator!=(const SampaHeader& rhs) const;
  bool operator<(const SampaHeader& rhs) const;
  bool operator<=(const SampaHeader& rhs) const;
  bool operator>(const SampaHeader& rhs) const;
  bool operator>=(const SampaHeader& rhs) const;

  uint8_t hammingCode() const;
  bool headerParity() const;
  SampaPacketType packetType() const;
  uint16_t nbOf10BitWords() const;
  uint8_t chipAddress() const;
  uint8_t channelAddress() const;
  uint32_t bunchCrossingCounter() const;
  bool payloadParity() const;

  void hammingCode(uint8_t hamming);
  void headerParity(bool p);
  void packetType(SampaPacketType pkt);
  void nbOf10BitWords(uint16_t nofwords);
  void chipAddress(uint8_t h);
  void channelAddress(uint8_t ch);
  void bunchCrossingCounter(uint32_t bx);
  void payloadParity(bool dp);

  uint64_t asUint64() const { return mValue; }

  bool isHeartbeat() const;

 public:
  uint64_t mValue;
};

SampaHeader sampaSync();

std::ostream& operator<<(std::ostream& os, const SampaHeader& sh);

} // namespace raw
} // namespace mch
} // namespace o2

#endif
