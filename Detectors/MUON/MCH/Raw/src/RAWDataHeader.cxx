#include "MCHRaw/RAWDataHeader.h"

#include <fmt/format.h>

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
