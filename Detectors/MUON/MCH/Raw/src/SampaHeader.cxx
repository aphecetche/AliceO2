#include "MCHRaw/SampaHeader.h"

#include <stdexcept>
#include <fmt/format.h>
#include <fmt/printf.h>
#include <iostream>
#include <vector>
#include <algorithm>

namespace
{
constexpr uint64_t HAMMING_CODE_OFFSET = 0;
constexpr uint64_t HEADER_PARITY_OFFSET = 6;
constexpr uint64_t PACKET_TYPE_OFFSET = 7;
constexpr uint64_t NUMBER_OF_1OBITS_WORDS_OFFSET = 10;
constexpr uint64_t CHIP_ADDRESS_OFFSET = 20;
constexpr uint64_t CHANNEL_ADDRESS_OFFSET = 24;
constexpr uint64_t BUNCH_CROSSING_OFFSET = 29;
constexpr uint64_t PARITY_OFFSET = 49;

constexpr uint64_t HAMMING_CODE_NOFBITS = HEADER_PARITY_OFFSET - HAMMING_CODE_OFFSET;
constexpr uint64_t HEADER_PARITY_NOFBITS = PACKET_TYPE_OFFSET - HEADER_PARITY_OFFSET;
constexpr uint64_t PACKET_TYPE_NOFBITS = NUMBER_OF_1OBITS_WORDS_OFFSET - PACKET_TYPE_OFFSET;
constexpr uint64_t NUMBER_OF_1OBITS_WORDS_NOFBITS = CHIP_ADDRESS_OFFSET - NUMBER_OF_1OBITS_WORDS_OFFSET;
constexpr uint64_t CHIP_ADDRESS_NOFBITS = CHANNEL_ADDRESS_OFFSET - CHIP_ADDRESS_OFFSET;
constexpr uint64_t CHANNEL_ADDRESS_NOFBITS = BUNCH_CROSSING_OFFSET - CHANNEL_ADDRESS_OFFSET;
constexpr uint64_t BUNCH_CROSSING_NOFBITS = PARITY_OFFSET - BUNCH_CROSSING_OFFSET;
constexpr uint64_t PARITY_NOFBITS = 50 - PARITY_OFFSET;

struct CHECKNOFBITS {

  CHECKNOFBITS()
  {
    if (HAMMING_CODE_NOFBITS != 6) {
      throw std::invalid_argument(fmt::format("HAMMING_CODE_NOFBITS is {0}. Should be 6", HAMMING_CODE_NOFBITS));
    }
    if (HEADER_PARITY_NOFBITS != 1) {
      throw std::invalid_argument(fmt::format("HEADER_PARITY_NOFBITS is {0}. Should be 1", HEADER_PARITY_NOFBITS));
    }
    if (PACKET_TYPE_NOFBITS != 3) {
      throw std::invalid_argument(fmt::format("PACKET_TYPE_NOFBITS is {0}. Should be 3", PACKET_TYPE_NOFBITS));
      throw;
    }
    if (NUMBER_OF_1OBITS_WORDS_NOFBITS != 10) {
      throw std::invalid_argument(fmt::format("NUMBER_OF_1OBITS_WORDS_NOFBITS is {0}. Should be 10", NUMBER_OF_1OBITS_WORDS_NOFBITS));
    }
    if (CHIP_ADDRESS_NOFBITS != 4) {
      throw std::invalid_argument(fmt::format("CHIP_ADDRESS_NOFBITS is {0}. Should be 4", CHIP_ADDRESS_NOFBITS));
    }
    if (CHANNEL_ADDRESS_NOFBITS != 5) {
      throw std::invalid_argument(fmt::format("CHANNEL_ADDRESS_NOFBITS is {0}. Should be 5", CHANNEL_ADDRESS_NOFBITS));
    }
    if (BUNCH_CROSSING_NOFBITS != 20) {
      throw std::invalid_argument(fmt::format("BUNCH_CROSSING_NOFBITS is {0}. Should be 20", BUNCH_CROSSING_NOFBITS));
    }
    if (PARITY_NOFBITS != 1) {
      throw std::invalid_argument(fmt::format("PARITY_NOFBITS is {0}. Should be 1", PARITY_NOFBITS));

      throw;
    }
  }
};

CHECKNOFBITS checknofbits;

constexpr uint64_t HAMMING_CODE_MASK = 0x000000000003F;
constexpr uint64_t HEADER_PARITY_MASK = 0x0000000000040;
constexpr uint64_t PACKET_TYPE_MASK = 0x0000000000380;
constexpr uint64_t NUMBER_OF_1OBITS_WORDS_MASK = 0x00000000FFC00;
constexpr uint64_t CHIP_ADDRESS_MASK = 0x0000000F00000;
constexpr uint64_t CHANNEL_ADDRESS_MASK = 0x000001F000000;
constexpr uint64_t BUNCH_CROSSING_MASK = 0x1FFFFE0000000;
constexpr uint64_t PARITY_MASK = 0x2000000000000;

constexpr uint64_t hammingCode(uint64_t header)
{
  return ((header & HAMMING_CODE_MASK) >> HAMMING_CODE_OFFSET);
}

constexpr uint64_t headerParity(uint64_t header)
{
  return ((header & HEADER_PARITY_MASK) >> HEADER_PARITY_OFFSET);
}
constexpr uint64_t packetType(uint64_t header)
{
  return ((header & PACKET_TYPE_MASK) >> PACKET_TYPE_OFFSET);
}

constexpr uint64_t nof10BitWords(uint64_t header)
{
  return ((header & NUMBER_OF_1OBITS_WORDS_MASK) >> NUMBER_OF_1OBITS_WORDS_OFFSET);
}

constexpr uint64_t chipAddress(uint64_t header)
{
  return ((header & CHIP_ADDRESS_MASK) >> CHIP_ADDRESS_OFFSET);
}
constexpr uint64_t channelAddress(uint64_t header)
{
  return ((header & CHANNEL_ADDRESS_MASK) >> CHANNEL_ADDRESS_OFFSET);
}

constexpr uint64_t bunchCrossingCounter(uint64_t header)
{
  return ((header & BUNCH_CROSSING_MASK) >> BUNCH_CROSSING_OFFSET);
}

constexpr uint64_t payloadParity(uint64_t header)
{
  return ((header & PARITY_MASK) >> PARITY_OFFSET);
}

template <typename T>
bool nofBitsBelowLimit(T value, int n)
{
  bool rv{true};
  uint64_t val{0};
  for (uint64_t i = n; i < sizeof(value) * 8; i++) {
    val |= (static_cast<T>(1) << i);
  }
  if (static_cast<uint64_t>(value) & val) {
    rv = false;
  }
  return rv;
}

template <typename T>
void assertNofBits(T value, int n)
{
  // throws an exception if value is not contained within n bits
  if (!nofBitsBelowLimit(value, n)) {
    throw std::invalid_argument(fmt::format("{0} should only be {1} bits-long",
                                            value, n));
  }
}

bool checkBit(uint64_t value, int i, bool bitStatus)
{
  return (value >> i & 1) == bitStatus;
}

void resetBits(uint64_t& value, int a, int n)
{
  for (int i = a; i < a + n; i++) {
    value &= ~(static_cast<uint64_t>(1) << i);
  }
}

bool isHeartbeat(uint64_t header)
{
  // - bits 7-9 must be zero
  // - bits 10-19 must be zero
  // - bits 24,26,28 must be one
  // - bits 25,27 must be zero
  // - bit 49 must be zero
  for (int i = 7; i <= 9; i++) {
    if (!checkBit(header, i, false)) {
      return false;
    }
  }
  for (int i = 10; i <= 19; i++) {
    if (!checkBit(header, i, false)) {
      return false;
    }
  }
  if (!checkBit(header, 24, true) || !checkBit(header, 26, true) || !checkBit(header, 28, true) || !checkBit(header, 25, false) || !checkBit(header, 27, false) || !checkBit(header, 49, false)) {
    return false;
  }
  return true;
}

} // namespace

namespace o2
{
namespace mch
{
namespace raw
{

std::string packetTypeName(o2::mch::raw::SampaPacketType pkt)
{
  if (pkt == o2::mch::raw::SampaPacketType::HeartBeat) {
    return "HeartBeat";
  }
  if (pkt == o2::mch::raw::SampaPacketType::DataTruncated) {
    return "DataTruncated";
  }
  if (pkt == o2::mch::raw::SampaPacketType::Sync) {
    return "Sync";
  }
  if (pkt == o2::mch::raw::SampaPacketType::DataTruncatedTriggerTooEarly) {
    return "DataTruncatedTriggerTooEarly";
  }
  if (pkt == o2::mch::raw::SampaPacketType::Data) {
    return "Data";
  }
  if (pkt == o2::mch::raw::SampaPacketType::DataNumWords) {
    return "DataNumWords";
  }
  if (pkt == o2::mch::raw::SampaPacketType::DataTriggerTooEarly) {
    return "DataTriggerTooEarl";
  }
  if (pkt == o2::mch::raw::SampaPacketType::DataTriggerTooEarlyNumWords) {
    return "DataTriggerTooEarlyNumWords";
  }
  throw std::out_of_range("should not happen");
}

SampaHeader::SampaHeader(uint64_t value) : mValue(value)
{
  if (!nofBitsBelowLimit(value, 50)) {
    throw std::invalid_argument(fmt::sprintf("%x is not a valid header value", value));
  }
}

SampaHeader::SampaHeader(uint8_t hamming,
                         bool p,
                         SampaPacketType pkt,
                         uint16_t numWords,
                         uint8_t h,
                         uint8_t ch,
                         uint32_t bx,
                         bool dp) : mValue{0}
{
  hammingCode(hamming);
  headerParity(p);
  packetType(pkt);
  nof10BitWords(numWords);
  chipAddress(h);
  channelAddress(ch);
  bunchCrossingCounter(bx);
  payloadParity(dp);
}

void SampaHeader::headerParity(bool p)
{
  resetBits(mValue, HEADER_PARITY_OFFSET, HEADER_PARITY_NOFBITS);
  mValue += (static_cast<uint64_t>(p) << HEADER_PARITY_OFFSET);
}

void SampaHeader::payloadParity(bool dp)
{
  resetBits(mValue, PARITY_OFFSET, PARITY_NOFBITS);
  mValue += (static_cast<uint64_t>(dp) << PARITY_OFFSET);
}

void SampaHeader::chipAddress(uint8_t h)
{
  assertNofBits(h, CHIP_ADDRESS_NOFBITS);
  resetBits(mValue, CHIP_ADDRESS_OFFSET, CHIP_ADDRESS_NOFBITS);
  mValue += (static_cast<uint64_t>(h) << CHIP_ADDRESS_OFFSET);
}

void SampaHeader::channelAddress(uint8_t ch)
{
  assertNofBits(ch, CHANNEL_ADDRESS_NOFBITS);
  resetBits(mValue, CHANNEL_ADDRESS_OFFSET, CHANNEL_ADDRESS_NOFBITS);
  mValue += (static_cast<uint64_t>(ch) << CHANNEL_ADDRESS_OFFSET);
}

void SampaHeader::bunchCrossingCounter(uint32_t bx)
{
  assertNofBits(bx, BUNCH_CROSSING_NOFBITS);
  resetBits(mValue, BUNCH_CROSSING_OFFSET, BUNCH_CROSSING_NOFBITS);
  mValue += (static_cast<uint64_t>(bx) << BUNCH_CROSSING_OFFSET);
}

void SampaHeader::hammingCode(uint8_t hamming)
{
  assertNofBits(hamming, HAMMING_CODE_NOFBITS);
  resetBits(mValue, HAMMING_CODE_OFFSET, HAMMING_CODE_NOFBITS);
  mValue += (static_cast<uint64_t>(hamming) << HAMMING_CODE_OFFSET);
}

void SampaHeader::nof10BitWords(uint16_t nofwords)
{
  assertNofBits(nofwords, NUMBER_OF_1OBITS_WORDS_NOFBITS);
  resetBits(mValue, NUMBER_OF_1OBITS_WORDS_OFFSET, NUMBER_OF_1OBITS_WORDS_NOFBITS);
  mValue += (static_cast<uint64_t>(nofwords) << NUMBER_OF_1OBITS_WORDS_OFFSET);
}

void SampaHeader::packetType(SampaPacketType pkt)
{
  resetBits(mValue, PACKET_TYPE_OFFSET, PACKET_TYPE_NOFBITS);
  mValue += (static_cast<uint64_t>(pkt) << PACKET_TYPE_OFFSET);
}

bool SampaHeader::operator<(const SampaHeader& rhs) const
{
  return bunchCrossingCounter() < rhs.bunchCrossingCounter();
}

bool SampaHeader::operator>(const SampaHeader& rhs) const
{
  return bunchCrossingCounter() > rhs.bunchCrossingCounter();
}

bool SampaHeader::operator<=(const SampaHeader& rhs) const
{
  return bunchCrossingCounter() <= rhs.bunchCrossingCounter();
}

bool SampaHeader::operator>=(const SampaHeader& rhs) const
{
  return bunchCrossingCounter() >= rhs.bunchCrossingCounter();
}

bool SampaHeader::operator==(const SampaHeader& rhs) const
{
  return mValue == rhs.mValue;
}

bool SampaHeader::operator!=(const SampaHeader& rhs) const
{
  return !(*this == rhs);
}

bool SampaHeader::isHeartbeat() const
{
  return ::isHeartbeat(mValue);
}
uint8_t SampaHeader::hammingCode() const
{
  // 6 bits
  return ::hammingCode(mValue) & 0X3F;
}

bool SampaHeader::headerParity() const
{
  return ::headerParity(mValue) == 1;
}

SampaPacketType SampaHeader::packetType() const
{
  // 3 bits
  return static_cast<SampaPacketType>(::packetType(mValue) & 0x7);
}

uint16_t SampaHeader::nof10BitWords() const
{
  // 10 bits
  return ::nof10BitWords(mValue) & 0x3FF;
}

uint8_t SampaHeader::chipAddress() const
{
  // 4 bits
  return ::chipAddress(mValue) & 0xF;
}

uint8_t SampaHeader::channelAddress() const
{
  // 5 bits
  return ::channelAddress(mValue) & 0x1F;
}

uint32_t SampaHeader::bunchCrossingCounter() const
{
  // 20 bits
  return ::bunchCrossingCounter(mValue) & 0x1FFFFF;
}

bool SampaHeader::payloadParity() const
{
  return ::payloadParity(mValue) == 1;
}

SampaHeader sampaSync()
{
  return SampaHeader(
    0x13,
    0,
    SampaPacketType::Sync,
    0,
    0xf,
    0,
    0xAAAAA,
    0);
}

std::ostream& operator<<(std::ostream& os, const SampaHeader& sh)
{
  uint64_t one = 1;
  std::vector<int> sep = {5, 6, 9, 19, 23, 28, 48};
  for (int i = 0; i < 50; i++) {
    os << ((sh.uint64() & (one << i)) ? "1" : "0");
    if (std::find(begin(sep), end(sep), i) != sep.end()) {
      os << "|";
    }
  }
  os << "\n";
  os << fmt::sprintf("%6x %d %3x %10x %4x %5x %20x %d [ %s ]",
                     sh.hammingCode(),
                     sh.headerParity(),
                     static_cast<uint8_t>(sh.packetType()),
                     sh.nof10BitWords(),
                     sh.chipAddress(),
                     sh.channelAddress(),
                     sh.bunchCrossingCounter(),
                     sh.payloadParity(),
                     packetTypeName(sh.packetType()))
     << "\n";
  return os;
}
} // namespace raw
} // namespace mch
} // namespace o2
