// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

///
/// @author  Laurent Aphecetche

#ifndef O2_MCH_SIMULATION_DETECTOR_H
#define O2_MCH_SIMULATION_DETECTOR_H

#include "DetectorsBase/Detector.h"
#include "SimulationDataFormat/BaseHits.h"
#include <iostream>

namespace o2
{
namespace mch
{

using MchHit = ::o2::BaseHit;

class Detector : public o2::Base::DetImpl<Detector>
{
 public:
  Detector(bool active = true);

  ~Detector() override = default;

  Bool_t ProcessHits(FairVolume* v = nullptr) override;

  void Initialize() override;

  void Register() override {}

  void Reset() override {}

  void ConstructGeometry() override;

  std::vector<MchHit>* getHits(int probe) { return (probe==0) ? &mHits : nullptr; }

 private:
  void defineSensitiveVolumes();

  std::vector<MchHit> mHits; //!
  float mEloss; //!

  ClassDefOverride(Detector, 1)
};

} // namespace mch
} // namespace o2

#endif
