// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_ENCODER_ELECTRONIC_MAPPER_H
#define O2_MCH_RAW_ENCODER_ELECTRONIC_MAPPER_H

#include <utility>
#include <set>

namespace o2::mch::raw
{

class ElectronicMapper
{
 public:
  /// GroupIdFromDeIdAndDsId returns the absolute / unique
  /// solarId and (elink) groupd Id corresponding to the pair
  /// (detection element id, dual sampa id)
  virtual std::pair<uint16_t, uint16_t>
    solarIdAndGroupIdFromDeIdAndDsId(uint16_t deid, uint16_t dsid) const = 0;

  /// solarIds returns the set of absolute/unique solarIds connected
  /// to the given cru
  virtual std::set<uint16_t> solarIds(uint8_t cruId) const = 0;

  /// cruIds returns the set of MCH CRU identifiers
  virtual std::set<uint16_t> cruIds() const = 0;

  virtual ~ElectronicMapper() = default;
};

struct ElectronicMapperGenerated {
};
struct ElectronicMapperFile {
};

template <typename T>
std::unique_ptr<ElectronicMapper> createElectronicMapper(T mapper);

} // namespace o2::mch::raw
#endif
