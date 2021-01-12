// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_CONDITIONS_DCS_NAMER_H
#define O2_MCH_CONDITIONS_DCS_NAMER_H

#include <vector>
#include <string>
#include <optional>

namespace o2::mch::dcs
{
enum class MeasurementType {
  Voltage,
  Current,
  Analog,
  Digital
};

enum class Side {
  Left,
  Right
};

struct ID {
  int number;
  Side side;
  int chamberId;
};

std::optional<ID> detElemId2DCS(int deId);

std::vector<std::string> aliases(std::vector<MeasurementType> types = {
                                   MeasurementType::Voltage,
                                   MeasurementType::Current,
                                   MeasurementType::Analog,
                                   MeasurementType::Digital});

} // namespace o2::mch::dcs

#endif
