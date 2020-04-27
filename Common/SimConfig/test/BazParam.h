// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include <CommonUtils/ConfigurableParamHelper.h>
#include <array>

using namespace o2;

class BazParam : public o2::conf::ConfigurableParamHelper<BazParam>
{
 public:
  double getGasDensity() const { return mGasDensity; }

 private:
  double mGasDensity = 42.42;
  //  std::array<int, 3> mPos = {1, 2, 3};

  O2ParamDef(BazParam, "Baz");
};
