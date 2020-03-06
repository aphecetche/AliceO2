// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_USER_LOGIC_ELINK_DECODER_DEBUG_H
#define O2_MCH_RAW_USER_LOGIC_ELINK_DECODER_DEBUG_H

#include <iostream>
#include "MCHRawElecMap/DsElecId.h"
#include <fmt/format.h>

namespace o2::mch::raw::ul
{
// uncomment this line to get a very verbose output
// and include it before the
// other includes that depends on it
#define ULDEBUG

template <typename FSM>
std::ostream& debugHeader(FSM& fsm)
{
  std::cout << fmt::format("{}--ULDEBUG--{:s}---------{:4d}--", reinterpret_cast<const void*>(&fsm), asString(fsm.dsId()), fsm.instance);
  return std::cout;
}

} // namespace o2::mch::raw::ul

#endif
