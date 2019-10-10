#include "MCHRaw/RAWDataHeader.h"

#include <fmt/format.h>
#include "Assertions.h"

std::ostream& operator<<(std::ostream& os, const o2::mch::raw::RAWDataHeader& rdh)
{
  os << fmt::format("version              {:03d} headerSize      {:03d} \n",
                    rdh.version,
                    rdh.headerSize);

  os << fmt::format("cruId                {:03d} dpwId            {:02d} linkId        {:03d}\n", rdh.cruId, rdh.dpwId, rdh.linkId);

  os << fmt::format("offsetNextPacket   {:05d} memorySize    {:05d} blockLength {:05d}\n", rdh.offsetNextPacket, rdh.memorySize, rdh.blockLength);

  os << fmt::format("triggerOrbit  {:010d} HB orbit {:010d}\n",
                    rdh.triggerOrbit, rdh.heartbeatOrbit);

  os << fmt::format("triggerBC           {:04d} heartbeatBC    {:04d}\n",
                    rdh.triggerBC, rdh.heartbeatBC);

  os << fmt::format("stopBit                {:1d} pagesCounter    {:03d} packetCounter {:03d} \n",
                    rdh.stopBit, rdh.pagesCounter, rdh.packetCounter);

  return os;
}

namespace o2
{
namespace mch
{
namespace raw
{

bool isValid(const RAWDataHeader& rdh)
{
  return rdh.version == 4 && rdh.headerSize == 64;
}

int forEachRDH(gsl::span<uint32_t> buffer, std::function<void(RAWDataHeader&)> f)
{
  o2::mch::raw::RAWDataHeader rdh;
  int index{0};
  int nrdh{0};
  while (index < buffer.size()) {
    memcpy(&rdh, &buffer[0] + index, sizeof(rdh));
    if (!isValid(rdh)) {
      break;
    }
    nrdh++;
    if (f) {
      f(rdh);
    }
    if (rdh.offsetNextPacket == 0) {
      return -1;
    }
    index += rdh.offsetNextPacket / 4;
  }
  return nrdh;
}

int countRDHs(gsl::span<uint32_t> buffer)
{
  return forEachRDH(buffer, nullptr);
}

using ::operator<<;

void dumpRDHBuffer(gsl::span<uint32_t> buffer)
{
  auto rdh = createRDH(buffer);
  std::cout << fmt::format("{:08X} {:08X} {:08X} {:08X}",
                           rdh.word3, rdh.word2, rdh.word1, rdh.word0);
  std::cout << fmt::format(" version {:d} headerSize {:d} blockLength {:d} \n",
                           rdh.version, rdh.headerSize, rdh.blockLength);
  std::cout << fmt::format("{:46s} feeId {} priority {}\n", " ", rdh.feeId, rdh.priorityBit);
  std::cout << fmt::format("{:46s} offsetnext {} memsize {}\n", " ", rdh.offsetNextPacket, rdh.memorySize);
  std::cout << fmt::format("{:46s} linkId {} packetCount {} cruId {} dpwId {}\n", " ", rdh.linkId, rdh.packetCounter, rdh.cruId, rdh.dpwId);

  std::cout << fmt::format("           {:08X} {:08X} {:08X} {:08X}",
                           rdh.word7, rdh.word6, rdh.word5, rdh.word4);
  std::cout << fmt::format(" triggerOrbit {:d} \n", rdh.triggerOrbit);
  std::cout << fmt::format("{:46s} heartbeatOrbit {}\n", " ", rdh.heartbeatOrbit);
  std::cout << fmt::format("{:46s} zero\n", " ");
  std::cout << fmt::format("{:46s} zero\n", " ");

  std::cout << fmt::format("           {:08X} {:08X} {:08X} {:08X}",
                           rdh.word11, rdh.word10, rdh.word9, rdh.word8);
  std::cout << fmt::format(" triggerBC {}  heartbeatBC {}\n", rdh.triggerBC,
                           rdh.heartbeatBC);
  std::cout << fmt::format("{:46s} triggerType {}\n", " ", rdh.triggerType);
  std::cout << fmt::format("{:46s} zero\n", " ");
  std::cout << fmt::format("{:46s} zero\n", " ");

  std::cout << fmt::format("           {:08X} {:08X} {:08X} {:08X}",
                           rdh.word15, rdh.word14, rdh.word13, rdh.word12);
  std::cout << fmt::format(" detectorField {}  par {}\n", rdh.detectorField,
                           rdh.par);
  std::cout << fmt::format("{:46s} stopBit {} pagesCounter {}\n", " ",
                           rdh.stopBit, rdh.pagesCounter);
  std::cout << fmt::format("{:46s} zero\n", " ");
  std::cout << fmt::format("{:46s} zero", " ");
}

void dumpBuffer(gsl::span<uint32_t> buffer)
{
  // dump a buffer, assuming it starts with a RDH
  // return the number of RDHs

  int i{0};
  while (i < buffer.size()) {
    if (i % 4 == 0) {
      std::cout << fmt::format("\n{:8d} : ", i * 4);
    }
    if (i + 16 <= buffer.size()) {
      auto rdh = createRDH(buffer.subspan(i));
      if (isValid(rdh)) {
        dumpRDHBuffer(buffer.subspan(i, 16));
        i += 16;
        continue;
      }
    }
    std::cout << fmt::format("{:08X} ", buffer[i]);
    i++;
  }
  std::cout << "\n";
}

int showRDHs(gsl::span<uint32_t> buffer)
{
  return forEachRDH(buffer, [](RAWDataHeader& rdh) {
    std::cout << rdh << "\n";
  });
}

void assertRDH(const RAWDataHeader& rdh)
{
  if (rdh.version != 4) {
    throw std::invalid_argument(fmt::format("RDH version {} is not the expected 4",
                                            rdh.version));
  }
  if (rdh.headerSize != 64) {
    throw std::invalid_argument(fmt::format("RDH size {} is not the expected 64",
                                            rdh.headerSize));
  }
}

void appendRDH(std::vector<uint32_t>& buffer, const RAWDataHeader& rdh)
{
  buffer.emplace_back(rdh.word3);
  buffer.emplace_back(rdh.word2);
  buffer.emplace_back(rdh.word1);
  buffer.emplace_back(rdh.word0);
  buffer.emplace_back(rdh.word7);
  buffer.emplace_back(rdh.word6);
  buffer.emplace_back(rdh.word5);
  buffer.emplace_back(rdh.word4);
  buffer.emplace_back(rdh.word11);
  buffer.emplace_back(rdh.word10);
  buffer.emplace_back(rdh.word9);
  buffer.emplace_back(rdh.word8);
  buffer.emplace_back(rdh.word15);
  buffer.emplace_back(rdh.word14);
  buffer.emplace_back(rdh.word13);
  buffer.emplace_back(rdh.word12);
}

RAWDataHeader createRDH(gsl::span<uint32_t> buffer)
{
  if (buffer.size() < 16) {
    throw std::invalid_argument("buffer should be at least 16 words");
  }
  RAWDataHeader rdh;
  int i{-1};
  rdh.word3 = buffer[++i];
  rdh.word2 = buffer[++i];
  rdh.word1 = buffer[++i];
  rdh.word0 = buffer[++i];
  rdh.word7 = buffer[++i];
  rdh.word6 = buffer[++i];
  rdh.word5 = buffer[++i];
  rdh.word4 = buffer[++i];
  rdh.word11 = buffer[++i];
  rdh.word10 = buffer[++i];
  rdh.word9 = buffer[++i];
  rdh.word8 = buffer[++i];
  rdh.word15 = buffer[++i];
  rdh.word14 = buffer[++i];
  rdh.word13 = buffer[++i];
  rdh.word12 = buffer[++i];

  return rdh;
}

RAWDataHeader createRDH(uint16_t cruId, uint8_t linkId, uint32_t orbit, uint16_t bunchCrossing,
                        uint16_t payloadSize)
{
  RAWDataHeader rdh;

  assertIsInRange("payloadSize", payloadSize, 0, 0xFFFF - sizeof(rdh));

  uint16_t memorySize = payloadSize + sizeof(rdh);

  rdh.cruId = cruId;
  rdh.linkId = linkId;
  rdh.dpwId = 0; // FIXME: fill this ?
  rdh.feeId = 0; //FIXME: what is this field supposed to contain ? unclear to me.
  rdh.priorityBit = 0;
  rdh.blockLength = memorySize - sizeof(rdh); // FIXME: the blockLength disappears in RDHv5 ?
  rdh.memorySize = memorySize;
  rdh.offsetNextPacket = memorySize;
  rdh.packetCounter = 0; // FIXME: fill this ?
  rdh.triggerType = 0;   // FIXME: fill this ?
  rdh.detectorField = 0; // FIXME: fill this ?
  rdh.par = 0;           // FIXME: fill this ?
  rdh.stopBit = 0;
  rdh.pagesCounter = 1;
  rdh.triggerOrbit = orbit;
  rdh.heartbeatOrbit = orbit; // FIXME: RDHv5 has only triggerOrbit ?
  rdh.triggerBC = bunchCrossing;
  rdh.heartbeatBC = bunchCrossing; // FIXME: RDHv5 has only triggerBC ?

  return rdh;
}

size_t rdhPayloadSize(const RAWDataHeader& rdh)
{
  size_t s = rdh.memorySize - sizeof(rdh);
  if (s != rdh.blockLength) {
    std::cout << rdh << "\n";
    std::cout << "memory size - " << sizeof(rdh) << " != " << rdh.blockLength
              << "\n";
  }
  assert(s == rdh.blockLength);
  return s;
}

} // namespace raw
} // namespace mch
} // namespace o2
