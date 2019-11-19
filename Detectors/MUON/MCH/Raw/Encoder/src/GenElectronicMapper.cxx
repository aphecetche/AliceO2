///
/// GENERATED CODE ! DO NOT EDIT !
///

#include <map>
#include <vector>
#include <set>
#include <utility>

#include "MCHRawEncoder/ElectronicMapper.h"
#include "Gench5.cxx"

namespace
{
std::map<uint16_t, uint16_t> createDeDsMap()
{
  std::map<uint16_t, uint16_t> m;
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
  m[1].insert(338);
  return m;
}
} // namespace

namespace o2::mch::raw
{

struct ElectronicMapperGeneratedImpl : public ElectronicMapper {

  std::pair<uint16_t, uint16_t>
    solarIdAndGroupIdFromDeIdAndDsId(uint16_t deid, uint16_t dsid)
      const override
  {
    static std::map<uint16_t, uint16_t> m = createDeDsMap();
    auto it = m.find(encode(deid, dsid));
    return *it;
  }

  std::set<uint16_t> solarIds(uint8_t cruId) const override
  {
    static std::map<uint16_t, std::set<uint16_t>> m = createCru2SolarMap();
    return m[cruId];
  }

  std::set<uint16_t> cruIds() const override
  {
    return {
      0,
      1,
    };
  }
};

template <>
std::unique_ptr<ElectronicMapper> createElectronicMapper(ElectronicMapperGenerated mapper)
{
  return std::make_unique<ElectronicMapperGeneratedImpl>();
}
} // namespace o2::mch::raw
