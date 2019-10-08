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
  rdh.blockLength = memorySize; // FIXME: the blockLength disappears in RDHv5 ?
  rdh.memorySize = memorySize;
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

} // namespace raw
} // namespace mch
} // namespace o2
