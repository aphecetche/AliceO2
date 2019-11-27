// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHRawEncoder/ElectronicMapper.h"
#include <map>
#include "ElectronicMappingImplHelper.h"
#include "MCHMappingFactory/CreateSegmentation.h"
#include <fmt/format.h>

using o2::mch::raw::impl::encodeDeDs;
using o2::mch::raw::impl::encodeSolarGroupIndex;

namespace o2::mch::raw
{

class ElectronicMapperDummyImpl : public ElectronicMapper
{
 public:
  ElectronicMapperDummyImpl();

  /// DualSampaElectronicLocation returns the absolute / unique
  /// solarId and (elink) groupd Id corresponding to the pair
  /// (detection element id, dual sampa id)
  virtual std::optional<DualSampaElectronicLocation>
    dualSampaElectronicLocation(uint16_t deId, uint16_t dsId) const override;

  /// solarIds returns the set of absolute/unique solarIds connected
  /// to the given cru
  virtual std::set<uint16_t> solarIds(uint8_t cruId) const override;

  /// cruId returns the identifier of the CRU connected to that detection element
  virtual std::optional<uint8_t> cruId(uint16_t deId) const override;

  /// cruIds returns the set of MCH CRU identifiers
  virtual std::set<uint16_t> cruIds() const override;

  virtual int nofSolars() const override { return mSolarIds.size(); }

 private:
  std::map<uint32_t, uint16_t> mDeIdDsId2SolarIdGroupIdIndex;
  std::map<uint16_t, uint8_t> mDeId2CruId;
  std::map<uint16_t, std::set<uint16_t>> mCruId2SolarId;
  std::set<uint16_t> mCruIds;
  std::set<uint16_t> mSolarIds;
};

ElectronicMapperDummyImpl::ElectronicMapperDummyImpl()
  : mDeIdDsId2SolarIdGroupIdIndex{},
    mDeId2CruId{},
    mCruId2SolarId{},
    mCruIds{}
{
  /// mDeIdDsId2SolarIdGroupIdIndex is a (fake) (deId,dsId)->(solarId,groupId,index) map
  ///
  /// to build this map we assume all solars have 8 groups (not true in reality)
  /// and that all groups have 5 dual sampas (i.e. index = 0..4) (which is
  /// also not true in reality).
  /// That way we end up with "only" 421 solars, while in reality we have
  /// more than 600.
  ///
  /// mDeId2CruId is a (fake) (deId)->(cruid) map
  ///
  /// one CRU can hold a maximum of 40*24=960 dual sampas
  /// to fill the fake map we simply loop over detection elements
  /// and use their number of dualsampas to compute a cruId.
  /// The one thing that closely ressembles reality is that any DE
  /// cannot be split between two CRUs. That way we end up with 19
  /// CRUs.

  constexpr int nofDualSampaPerCRU = 40 * 24;
  uint16_t n{0};
  uint16_t solarId{0};
  uint8_t groupId{0};
  uint8_t index{0};
  uint8_t cruId{0};
  int nofDualSampas{0};

  std::set<int> solars;

  o2::mch::mapping::forEachDetectionElement([&](int deId) {
    auto seg = o2::mch::mapping::segmentation(deId);
    // assign a cruId to this deId
    int nds = seg.nofDualSampas();
    if ((nds + nofDualSampas) < nofDualSampaPerCRU) {
      nofDualSampas += nds;
    } else {
      // this CRU is "full"
      nofDualSampas = nds;
      n = 0;
      cruId++;
    }
    mDeId2CruId[deId] = cruId;
    mCruIds.insert(cruId);
    // assign a tuple (solarId,groupId,index) to the pair (deId,dsId)
    seg.forEachDualSampa([&](int dsId) {
      // index 0..4
      // groupId 0..7
      // solarId 0..nsolars
      if (n % 5 == 0) {
        index = 0;
        if (n % 8 == 0) {
          groupId = 0;
        } else {
          groupId++;
        }
      } else {
        index++;
      }
      if (n % 40 == 0) {
        solarId++;
      }
      mCruId2SolarId[cruId].insert(solarId);
      mDeIdDsId2SolarIdGroupIdIndex.insert(std::make_pair(encodeDeDs(deId, dsId), encodeSolarGroupIndex(solarId, groupId, index)));
      n++;
      mSolarIds.insert(solarId);
      // std::cout << fmt::format(
      //   "DE {:4d} CRU {:d} DS {:4d} SOLAR {:4d} "
      //   "GROUP {:d} index {:d}\n",
      //   deId, cruId, dsId, solarId, groupId, index);
    });
  });
}

std::optional<DualSampaElectronicLocation> ElectronicMapperDummyImpl::dualSampaElectronicLocation(uint16_t deId, uint16_t dsId) const
{
  auto it = mDeIdDsId2SolarIdGroupIdIndex.find(encodeDeDs(deId, dsId));
  if (it == mDeIdDsId2SolarIdGroupIdIndex.end()) {
    return std::nullopt;
  }
  return DualSampaElectronicLocation{impl::decodeSolarId(it->second), impl::decodeGroupId(it->second), impl::decodeElinkIndex(it->second)};
}

std::set<uint16_t> ElectronicMapperDummyImpl::solarIds(uint8_t cruId) const
{
  auto it = mCruId2SolarId.find(cruId);
  if (it == mCruId2SolarId.end()) {
    return {};
  }
  return it->second;
}

/// cruId returns the identifier of the CRU connected to that detection element
std::optional<uint8_t> ElectronicMapperDummyImpl::cruId(uint16_t deId) const
{
  auto it = mDeId2CruId.find(deId);
  if (it == mDeId2CruId.end()) {
    return std::nullopt;
  }
  return it->second;
}

std::set<uint16_t> ElectronicMapperDummyImpl::cruIds() const
{
  return mCruIds;
}

template <>
std::unique_ptr<ElectronicMapper> createElectronicMapper<ElectronicMapperDummy>()
{
  return std::make_unique<ElectronicMapperDummyImpl>();
}
} // namespace o2::mch::raw
