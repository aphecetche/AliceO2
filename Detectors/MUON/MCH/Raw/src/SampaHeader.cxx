#include "MCHRaw/SampaHeader.h"

#include <stdexcept>
#include <fmt/format.h>
#include <fmt/printf.h>
#include <iostream>

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

constexpr uint64_t nbOf10BitWords(uint64_t header)
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
  return ((header & BUNCH_CROSSING_OFFSET) >> BUNCH_CROSSING_OFFSET);
}

constexpr uint64_t payloadParity(uint64_t header)
{
  return ((header & PARITY_MASK) >> PARITY_OFFSET);
}

constexpr uint64_t sampaHeader(uint64_t hamming,
                               uint64_t p,
                               uint64_t pkt,
                               uint64_t numWords,
                               uint64_t h,
                               uint64_t ch,
                               uint64_t bx,
                               uint64_t dp)

{
  // 50 bits

  return (hamming << HAMMING_CODE_OFFSET) +
           (p << HEADER_PARITY_OFFSET) +
           (pkt << PACKET_TYPE_OFFSET) +
           (numWords << NUMBER_OF_1OBITS_WORDS_OFFSET) +
           (h << CHIP_ADDRESS_OFFSET) +
           (ch << CHANNEL_ADDRESS_OFFSET) +
           (bx << BUNCH_CROSSING_OFFSET) +
           (dp << PARITY_OFFSET) &
         UINT64_C(0x3FFFFFFFFFFFF);
}

constexpr uint64_t sampaSync()
{
  return sampaHeader(
    0x13,
    0,
    2,
    0,
    0xf,
    0,
    0xAAAAA,
    0);
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

bool checkBit(uint64_t value, int i, bool bitStatus, std::string& error)
{
  bool rv = (value >> i & 1) == bitStatus;
  if (!rv) {
    error += fmt::sprintf("bit %2d should be %d\n", i, bitStatus);
  }
  return rv;
}

bool isValidHeader(uint64_t header, std::string& error)
{
  if (!nofBitsBelowLimit(header, 50)) {
    return false;
  }
  // - bits 7-9 must be zero
  // - bits 24,26,28 must be one
  // - bits 25,27 must be zero
  // - bit 49 must be zero
  checkBit(header, 7, false, error);
  checkBit(header, 8, false, error);
  checkBit(header, 9, false, error);
  checkBit(header, 24, true, error);
  checkBit(header, 26, true, error);
  checkBit(header, 28, true, error);
  checkBit(header, 25, false, error);
  checkBit(header, 27, false, error);
  checkBit(header, 49, false, error);
  return error.empty();
}

} // namespace

namespace o2
{
namespace mch
{
namespace raw
{

SampaHeader::SampaHeader(uint64_t value) : mValue(value)
{
  std::string error;
  if (!isValidHeader(value, error)) {
    throw std::invalid_argument(fmt::sprintf("%x is not a valid header value :\n%s", value, error));
  }
}

SampaHeader::SampaHeader(uint8_t hamming,
                         bool p,
                         uint8_t pkt,
                         uint16_t numWords,
                         uint8_t h,
                         uint8_t ch,
                         uint32_t bx,
                         bool dp) : mValue{0}
{
  assertNofBits(hamming, 6);
  assertNofBits(pkt, 3);
  assertNofBits(numWords, 10);
  assertNofBits(h, 4);
  assertNofBits(ch, 5);
  assertNofBits(bx, 20);
  mValue = (hamming << HAMMING_CODE_OFFSET) +
           (static_cast<uint64_t>(p) << HEADER_PARITY_OFFSET) +
           (pkt << PACKET_TYPE_OFFSET) +
           (numWords << NUMBER_OF_1OBITS_WORDS_OFFSET) +
           (h << CHIP_ADDRESS_OFFSET) +
           (ch << CHANNEL_ADDRESS_OFFSET) +
           (bx << BUNCH_CROSSING_OFFSET) +
           (static_cast<uint64_t>(dp) << PARITY_OFFSET);
  std::string error;
  if (!isValidHeader(mValue, error)) {
    throw std::invalid_argument(fmt::sprintf("%x is not a valid header value :\n%s", mValue, error));
  }
}

bool SampaHeader::operator==(const SampaHeader& rhs) const
{
  return mValue == rhs.mValue;
}

bool SampaHeader::operator!=(const SampaHeader& rhs) const
{
  return !(*this == rhs);
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

uint8_t SampaHeader::packetType() const
{
  // 3 bits
  return ::packetType(mValue) & 0x7;
}

uint16_t SampaHeader::nbOf10BitWords() const
{
  // 10 bits
  return ::nbOf10BitWords(mValue) & 0x3FF;
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
    2,
    0,
    0xf,
    0,
    0xAAAAA,
    0);
}

} // namespace raw
} // namespace mch
} // namespace o2
