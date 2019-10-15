#include "MCHRaw/SampaHeader.h"

#include <stdexcept>
#include <fmt/format.h>
#include <fmt/printf.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include "NofBits.h"

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

SampaHeader::SampaHeader(uint64_t value) : mValue(0)
{
  uint64(value);
}

bool SampaHeader::hasError() const
{
  unsigned int buf[2] = {
    static_cast<unsigned int>(mValue & 0x3FFFFFFF),
    static_cast<unsigned int>(mValue >> 30)};
  bool hamming_error = false;  // Is there an hamming error?
  bool hamming_uncorr = false; // Is the data correctable?
  bool hamming_enable = false; // Correct the data?
  hammingDecode(buf, hamming_error, hamming_uncorr, hamming_enable);
  return hamming_error;
}

void SampaHeader::uint64(uint64_t value)
{
  assertNofBits("sampa header", value, 50);
  mValue = value;
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
  assertNofBits("chip", h, CHIP_ADDRESS_NOFBITS);
  resetBits(mValue, CHIP_ADDRESS_OFFSET, CHIP_ADDRESS_NOFBITS);
  mValue += (static_cast<uint64_t>(h) << CHIP_ADDRESS_OFFSET);
}

void SampaHeader::channelAddress(uint8_t ch)
{
  assertNofBits("ch", ch, CHANNEL_ADDRESS_NOFBITS);
  resetBits(mValue, CHANNEL_ADDRESS_OFFSET, CHANNEL_ADDRESS_NOFBITS);
  mValue += (static_cast<uint64_t>(ch) << CHANNEL_ADDRESS_OFFSET);
}

void SampaHeader::bunchCrossingCounter(uint32_t bx)
{
  // assertNofBits("bx", bx, BUNCH_CROSSING_NOFBITS);
  // resetBits(mValue, BUNCH_CROSSING_OFFSET, BUNCH_CROSSING_NOFBITS);
  mValue &= ~BUNCH_CROSSING_MASK;
  mValue += (static_cast<uint64_t>(bx) << BUNCH_CROSSING_OFFSET) & BUNCH_CROSSING_MASK;
}

void SampaHeader::hammingCode(uint8_t hamming)
{
  assertNofBits("hamming", hamming, HAMMING_CODE_NOFBITS);
  resetBits(mValue, HAMMING_CODE_OFFSET, HAMMING_CODE_NOFBITS);
  mValue += (static_cast<uint64_t>(hamming) << HAMMING_CODE_OFFSET);
}

void SampaHeader::nof10BitWords(uint16_t nofwords)
{
  // assertNofBits("nof10BitWords", nofwords, NUMBER_OF_1OBITS_WORDS_NOFBITS);
  // resetBits(mValue, NUMBER_OF_1OBITS_WORDS_OFFSET, NUMBER_OF_1OBITS_WORDS_NOFBITS);
  mValue &= ~NUMBER_OF_1OBITS_WORDS_MASK;
  mValue += (static_cast<uint64_t>(nofwords) << NUMBER_OF_1OBITS_WORDS_OFFSET) & NUMBER_OF_1OBITS_WORDS_MASK;
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
  os << fmt::sprintf("%6x %d %3x %10x %4x %5x %20x %d (0x%x) | %s %s",
                     sh.hammingCode(),
                     sh.headerParity(),
                     static_cast<uint8_t>(sh.packetType()),
                     sh.nof10BitWords(),
                     sh.chipAddress(),
                     sh.channelAddress(),
                     sh.bunchCrossingCounter(),
                     sh.payloadParity(),
                     sh.uint64(),
                     packetTypeName(sh.packetType()),
                     sh.hasError() ? "ERROR" : "")
     << "\n";
  return os;
}

void hammingDecode(unsigned int buffer[2], bool& error, bool& uncorrectable, bool fix_data)
//
// From Arild Velure code
//
{

  // header split
  bool parityreceived[6];
  bool data_in[43];
  bool overallparity;

  for (int i = 0; i < 6; i++)
    parityreceived[i] = (buffer[0] >> i) & 0x1;
  overallparity = (buffer[0] >> 6) & 0x1;
  //for (int i = 0; i < 43; i++)
  //  data_in[i] = (header_in >> (i + 7)) & 0x1;
  for (int i = 7; i < 30; i++)
    data_in[i - 7] = (buffer[0] >> i) & 0x1;
  for (int i = 30; i < 50; i++)
    data_in[i - 7] = (buffer[1] >> (i - 30)) & 0x1;

  //calculated values
  bool corrected_out[43];
  bool overallparitycalc = 0;
  bool overallparity_out = 0;
  bool paritycalc[6];
  bool paritycorreced_out[6];

  ////////////////////////////////////////////////////////////////////////////////////////////////
  // calculate parity
  paritycalc[0] = data_in[0] ^ data_in[1] ^ data_in[3] ^ data_in[4] ^ data_in[6] ^
                  data_in[8] ^ data_in[10] ^ data_in[11] ^ data_in[13] ^ data_in[15] ^
                  data_in[17] ^ data_in[19] ^ data_in[21] ^ data_in[23] ^ data_in[25] ^
                  data_in[26] ^ data_in[28] ^ data_in[30] ^ data_in[32] ^ data_in[34] ^
                  data_in[36] ^ data_in[38] ^ data_in[40] ^ data_in[42];

  paritycalc[1] = data_in[0] ^ data_in[2] ^ data_in[3] ^ data_in[5] ^ data_in[6] ^
                  data_in[9] ^ data_in[10] ^ data_in[12] ^ data_in[13] ^ data_in[16] ^
                  data_in[17] ^ data_in[20] ^ data_in[21] ^ data_in[24] ^ data_in[25] ^
                  data_in[27] ^ data_in[28] ^ data_in[31] ^ data_in[32] ^ data_in[35] ^
                  data_in[36] ^ data_in[39] ^ data_in[40];

  paritycalc[2] = data_in[1] ^ data_in[2] ^ data_in[3] ^ data_in[7] ^ data_in[8] ^
                  data_in[9] ^ data_in[10] ^ data_in[14] ^ data_in[15] ^ data_in[16] ^
                  data_in[17] ^ data_in[22] ^ data_in[23] ^ data_in[24] ^ data_in[25] ^
                  data_in[29] ^ data_in[30] ^ data_in[31] ^ data_in[32] ^ data_in[37] ^
                  data_in[38] ^ data_in[39] ^ data_in[40];

  paritycalc[3] = data_in[4] ^ data_in[5] ^ data_in[6] ^ data_in[7] ^ data_in[8] ^
                  data_in[9] ^ data_in[10] ^ data_in[18] ^ data_in[19] ^ data_in[20] ^
                  data_in[21] ^ data_in[22] ^ data_in[23] ^ data_in[24] ^ data_in[25] ^
                  data_in[33] ^ data_in[34] ^ data_in[35] ^ data_in[36] ^ data_in[37] ^
                  data_in[38] ^ data_in[39] ^ data_in[40];

  paritycalc[4] = data_in[11] ^ data_in[12] ^ data_in[13] ^ data_in[14] ^ data_in[15] ^
                  data_in[16] ^ data_in[17] ^ data_in[18] ^ data_in[19] ^ data_in[20] ^
                  data_in[21] ^ data_in[22] ^ data_in[23] ^ data_in[24] ^ data_in[25] ^
                  data_in[41] ^ data_in[42];

  paritycalc[5] = data_in[26] ^ data_in[27] ^ data_in[28] ^ data_in[29] ^ data_in[30] ^
                  data_in[31] ^ data_in[32] ^ data_in[33] ^ data_in[34] ^ data_in[35] ^
                  data_in[36] ^ data_in[37] ^ data_in[38] ^ data_in[39] ^ data_in[40] ^
                  data_in[41] ^ data_in[42];
  ////////////////////////////////////////////////////////////////////////////////////////////////

  //    uint8_t syndrome = 0;
  unsigned char syndrome = 0;

  for (int i = 0; i < 6; i++)
    syndrome |= (paritycalc[i] ^ parityreceived[i]) << i;

  bool data_parity_interleaved[64];
  bool syndromeerror;

  //data_parity_interleaved[0]          =  0;
  data_parity_interleaved[1] = parityreceived[0];
  data_parity_interleaved[2] = parityreceived[1];
  data_parity_interleaved[3] = data_in[0];
  data_parity_interleaved[4] = parityreceived[2];
  for (int i = 1; i <= 3; i++)
    data_parity_interleaved[i + 5 - 1] = data_in[i];
  data_parity_interleaved[8] = parityreceived[3];
  for (int i = 4; i <= 10; i++)
    data_parity_interleaved[i + 9 - 4] = data_in[i];
  data_parity_interleaved[16] = parityreceived[4];
  for (int i = 11; i <= 25; i++)
    data_parity_interleaved[i + 17 - 11] = data_in[i];
  data_parity_interleaved[32] = parityreceived[5];
  for (int i = 26; i <= 42; i++)
    data_parity_interleaved[i + 33 - 26] = data_in[i];
  //for (int i = 50; i <= 63; i++)
  //  data_parity_interleaved[i]        =  0;

  data_parity_interleaved[syndrome] = !data_parity_interleaved[syndrome]; // correct the interleaved

  paritycorreced_out[0] = data_parity_interleaved[1];
  paritycorreced_out[1] = data_parity_interleaved[2];
  corrected_out[0] = data_parity_interleaved[3];
  paritycorreced_out[2] = data_parity_interleaved[4];
  for (int i = 1; i <= 3; i++)
    corrected_out[i] = data_parity_interleaved[i + 5 - 1];
  paritycorreced_out[3] = data_parity_interleaved[8];
  for (int i = 4; i <= 10; i++)
    corrected_out[i] = data_parity_interleaved[i + 9 - 4];
  paritycorreced_out[4] = data_parity_interleaved[16];
  for (int i = 11; i <= 25; i++)
    corrected_out[i] = data_parity_interleaved[i + 17 - 11];
  paritycorreced_out[5] = data_parity_interleaved[32];
  for (int i = 26; i <= 42; i++)
    corrected_out[i] = data_parity_interleaved[i + 33 - 26];

  // now we have the "corrected" data -> update the flags

  bool wrongparity;
  for (int i = 0; i < 43; i++)
    overallparitycalc ^= data_in[i];
  for (int i = 0; i < 6; i++)
    overallparitycalc ^= parityreceived[i];
  syndromeerror = (syndrome > 0) ? 1 : 0; // error if syndrome larger than 0
  wrongparity = (overallparitycalc != overallparity);
  overallparity_out = !syndromeerror && wrongparity ? overallparitycalc : overallparity; // If error was in parity fix parity
  error = syndromeerror | wrongparity;
  uncorrectable = (syndromeerror && (!wrongparity));

  //header_out = 0;
  //for (int i = 0; i < 43; i++)
  //  header_out |= corrected_out[i] << (i + 7);
  //header_out |= overallparity_out << 6;
  //for (int i = 0; i < 6; i++)
  //  header_out |= paritycorreced_out[i] << i;
  if (fix_data) {
    for (int i = 0; i < 6; i++)
      buffer[0] = (buffer[0] & ~(1 << i)) | (paritycorreced_out[i] << i);
    buffer[0] = (buffer[0] & ~(1 << 6)) | (overallparity_out << 6);
    for (int i = 7; i < 30; i++)
      buffer[0] = (buffer[0] & ~(1 << i)) | (corrected_out[i - 7] << i);
    for (int i = 30; i < 50; i++)
      buffer[1] = (buffer[1] & ~(1 << (i - 30))) | (corrected_out[i - 7] << (i - 30));
  }
}

} // namespace raw
} // namespace mch
} // namespace o2
