// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_ELECMAP_DS_DET_ID_H
#define O2_MCH_RAW_ELECMAP_DS_DET_ID_H

#include <cstdint>
#include <iosfwd>

namespace o2::mch::raw
{
class DsDetId
{
 public:
  DsDetId(int deId, int dsId);

  uint16_t deId() const { return mDeId; }
  uint16_t dsId() const { return mDsId; }

 private:
  uint16_t mDeId;
  uint16_t mDsId;
};

DsDetId decodeDsDetId(uint32_t code);
uint32_t encode(const DsDetId& id);

std::ostream& operator<<(std::ostream& os, const DsDetId& id);

} // namespace o2::mch::raw
#endif
