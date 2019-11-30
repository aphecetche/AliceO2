///
/// GENERATED CODE ! DO NOT EDIT !
///

#include <map>
#include <vector>
#include <set>
#include <utility>

#include "MCHRawElecMap/Mapper.h"

#include "ImplHelper.h"

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
  m[1].insert(186);
  return m;
}
} // namespace

std::map<uint16_t, uint8_t> createDeId2CruIdMap()
{
  std::map<uint16_t, uint8_t> m;
  m[505] = 0;
  m[501] = 1;
  m[500] = 1;
  return m;
}

namespace o2::mch::raw
{

struct ElectronicMapperGeneratedImpl : public ElectronicMapper {

  std::optional<DualSampaElectronicLocation>
    dualSampaElectronicLocation(uint16_t deid, uint16_t dsid)
      const override
  {
    static std::map<uint32_t, uint16_t> m = createDeDsMap();
    auto it = m.find(encodeDeDs(deid, dsid));
    if (it == m.end()) {
      return std::nullopt;
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

  std::optional<uint8_t> cruId(uint16_t deid) const override
  {
    static std::map<uint16_t, uint8_t> m = createDeId2CruIdMap();
    auto it = m.find(deid);
    if (it == m.end()) {
      return std::nullopt;
    }
    return m[deid];
  }

  std::set<uint16_t> cruIds() const override
  {
    return {
      0,
      1,
    };
  }
  int nofSolars() const override { return 9; }
};
template <>
std::unique_ptr<ElectronicMapper> createElectronicMapper<ElectronicMapperGenerated>()
{
  return std::make_unique<ElectronicMapperGeneratedImpl>();
}
} // namespace o2::mch::raw