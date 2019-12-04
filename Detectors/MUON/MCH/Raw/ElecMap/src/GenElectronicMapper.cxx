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
  m[nan].insert(344);
  m[nan].insert(344);
  m[nan].insert(344);
  m[nan].insert(344);
  m[nan].insert(344);
  m[nan].insert(344);
  m[nan].insert(344);
  m[nan].insert(344);
  m[nan].insert(345);
  m[nan].insert(345);
  m[nan].insert(345);
  m[nan].insert(345);
  m[nan].insert(345);
  m[nan].insert(345);
  m[nan].insert(346);
  m[nan].insert(346);
  m[nan].insert(346);
  m[nan].insert(346);
  m[nan].insert(346);
  m[nan].insert(346);
  m[nan].insert(347);
  m[nan].insert(347);
  m[nan].insert(347);
  m[nan].insert(347);
  m[nan].insert(347);
  m[nan].insert(348);
  m[nan].insert(348);
  m[nan].insert(348);
  m[nan].insert(348);
  m[nan].insert(348);
  m[nan].insert(304);
  m[nan].insert(304);
  m[nan].insert(304);
  m[nan].insert(304);
  m[nan].insert(304);
  m[nan].insert(305);
  m[nan].insert(305);
  m[nan].insert(305);
  m[nan].insert(305);
  m[nan].insert(305);
  m[nan].insert(306);
  m[nan].insert(306);
  m[nan].insert(306);
  m[nan].insert(306);
  m[nan].insert(306);
  m[nan].insert(306);
  m[nan].insert(306);
  m[nan].insert(307);
  m[nan].insert(307);
  m[nan].insert(307);
  m[nan].insert(307);
  m[nan].insert(307);
  m[nan].insert(307);
  m[nan].insert(308);
  m[nan].insert(308);
  m[nan].insert(308);
  m[nan].insert(308);
  m[nan].insert(308);
  m[nan].insert(309);
  m[nan].insert(309);
  m[nan].insert(309);
  m[nan].insert(309);
  m[nan].insert(309);
  m[nan].insert(352);
  m[nan].insert(352);
  m[nan].insert(352);
  m[nan].insert(352);
  m[nan].insert(352);
  m[nan].insert(353);
  m[nan].insert(353);
  m[nan].insert(353);
  m[nan].insert(353);
  m[nan].insert(353);
  m[nan].insert(354);
  m[nan].insert(354);
  m[nan].insert(354);
  m[nan].insert(354);
  m[nan].insert(354);
  m[nan].insert(355);
  m[nan].insert(355);
  m[nan].insert(355);
  m[nan].insert(355);
  m[nan].insert(355);
  m[nan].insert(356);
  m[nan].insert(356);
  m[nan].insert(356);
  m[nan].insert(356);
  m[nan].insert(356);
  m[nan].insert(357);
  m[nan].insert(357);
  m[nan].insert(357);
  m[nan].insert(357);
  m[nan].insert(357);
  m[nan].insert(448);
  m[nan].insert(448);
  m[nan].insert(448);
  m[nan].insert(448);
  m[nan].insert(448);
  m[nan].insert(448);
  m[nan].insert(448);
  m[nan].insert(449);
  m[nan].insert(449);
  m[nan].insert(449);
  m[nan].insert(449);
  m[nan].insert(449);
  m[nan].insert(449);
  m[nan].insert(450);
  m[nan].insert(450);
  m[nan].insert(450);
  m[nan].insert(450);
  m[nan].insert(450);
  m[nan].insert(451);
  m[nan].insert(451);
  m[nan].insert(451);
  m[nan].insert(451);
  m[nan].insert(451);
  m[nan].insert(452);
  m[nan].insert(452);
  m[nan].insert(452);
  m[nan].insert(452);
  m[nan].insert(452);
  m[nan].insert(453);
  m[nan].insert(453);
  m[nan].insert(453);
  m[nan].insert(453);
  m[nan].insert(453);
  m[nan].insert(368);
  m[nan].insert(368);
  m[nan].insert(368);
  m[nan].insert(368);
  m[nan].insert(368);
  m[nan].insert(368);
  m[nan].insert(369);
  m[nan].insert(369);
  m[nan].insert(369);
  m[nan].insert(369);
  m[nan].insert(369);
  m[nan].insert(369);
  m[nan].insert(370);
  m[nan].insert(370);
  m[nan].insert(370);
  m[nan].insert(370);
  m[nan].insert(370);
  m[nan].insert(370);
  m[nan].insert(370);
  m[nan].insert(370);
  return m;
}
} // namespace

std::map<uint16_t, uint8_t> createDeId2CruIdMap()
{
  std::map<uint16_t, uint8_t> m;
  m[604] = nan;
  m[603] = nan;
  m[602] = nan;
  m[601] = nan;
  m[600] = nan;
  m[617] = nan;
  m[616] = nan;
  m[615] = nan;
  m[614] = nan;
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
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
      nan,
    };
  }
  int nofSolars() const override { return 26; }
};
template <>
std::unique_ptr<ElectronicMapper> createElectronicMapper<ElectronicMapperGenerated>()
{
  return std::make_unique<ElectronicMapperGeneratedImpl>();
}
} // namespace o2::mch::raw