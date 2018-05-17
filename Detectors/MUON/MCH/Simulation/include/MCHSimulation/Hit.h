// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_SIMULATION_HIT_H
#define O2_MCH_SIMULATION_HIT_H

#include "SimulationDataFormat/BaseHits.h"

namespace o2
{
namespace mch
{
struct Hit : public ::o2::BaseHit {

  Hit(int trackId = 0, short detElemId = 0, float eloss = 0.0, float length = 0.0)
    : ::o2::BaseHit(trackId), mDetElemId{ detElemId }, mEloss{ eloss }, mLength{ length }
  {
  }

  short mDetElemId{0};
  float mEloss{0.0}; 
  float mLength{0.0};

  ClassDefNV(Hit,1);
};

} // namespace mch
} // namespace o2

#endif