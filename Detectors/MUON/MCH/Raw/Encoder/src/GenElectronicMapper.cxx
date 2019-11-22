///
/// GENERATED CODE ! DO NOT EDIT !
///

#include <map>
#include <vector>
#include <set>
#include <utility>

#include "MCHRawEncoder/ElectronicMapper.h"

uint32_t encodeDeDs(uint16_t a, uint16_t b)
{
  return a << 16 | b;
}
uint16_t decode_a(uint32_t x)
{
  return static_cast<uint16_t>((x & 0xFFFF0000) >> 16);
}
uint16_t decode_b(uint32_t x)
{
  return static_cast<uint16_t>(x & 0xFFFF);
}

uint16_t encodeSolarGroupIndex(uint16_t solarId, uint8_t groupId, uint8_t index)
{
  return (solarId & 0x3FF) | ((groupId & 0x7) << 10) |
         ((index & 0x7) << 13);
}

uint16_t decodeSolarId(uint16_t code)
{
  return code & 0x3FF;
}

uint8_t decodeGroupId(uint16_t code)
{
  return (code & 0x1C00) >> 10;
}

uint8_t decodeElinkIndex(uint16_t code)
{
  return (code & 0xE000) >> 13;
}
#include "Gench5.cxx"

namespace
{
std::map<uint32_t, uint16_t> createDeDsMap()
{
  std::map<uint32_t, uint16_t> m;
  fillch5(m);
  return m;
}
} // namespace
namespace
{
std::map<uint16_t, std::set<uint16_t>> createCru2SolarMap()
{
  std::map<uint16_t, std::set<uint16_t>> m;
  m[0].insert(144);
  return m;
}
} // namespace

std::map<uint16_t, uint8_t> createDeId2CruIdMap()
{
  std::map<uint16_t, uint8_t> m;
  m[505] = 0;
  return m;
}

namespace o2::mch::raw
{

struct ElectronicMapperGeneratedImpl : public ElectronicMapper {

  DualSampaElectronicLocation
    dualSampaElectronicLocation(uint16_t deid, uint16_t dsid)
      const override
  {
    static std::map<uint32_t, uint16_t> m = createDeDsMap();
    auto it = m.find(encodeDeDs(deid, dsid));
    if (it == m.end()) {
      return DualSampaElectronicLocation::Invalid();
    }
    return DualSampaElectronicLocation{decodeSolarId(it->second), decodeGroupId(it->second), decodeElinkIndex(it->second)};
  }

  std::set<uint16_t> solarIds(uint8_t cruId) const override
  {
    static std::map<uint16_t, std::set<uint16_t>> m = createCru2SolarMap();
    auto it = m.find(cruId);
    if (it == m.end()) {
      return {};
    }
    return m[cruId];
  }

  uint8_t cruId(uint16_t deid) const override
  {
    static std::map<uint16_t, uint8_t> m = createDeId2CruIdMap();
    auto it = m.find(deid);
    if (it == m.end()) {
      return 0xFF;
    }
    return m[deid];
  }

  std::set<uint16_t> cruIds() const override
  {
    return {
      0,
    };
  }
};
template <>
std::unique_ptr<ElectronicMapper> createElectronicMapper<ElectronicMapperGenerated>()
{
  return std::make_unique<ElectronicMapperGeneratedImpl>();
}
} // namespace o2::mch::raw