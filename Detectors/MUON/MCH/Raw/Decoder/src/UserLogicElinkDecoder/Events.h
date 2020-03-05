// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_USER_LOGIC_ELINK_EVENTS_H
#define O2_MCH_RAW_USER_LOGIC_ELINK_EVENTS_H

#include <cstdlib>
#include <string>

namespace o2::mch::raw
{
// Events
constexpr uint64_t FIFTYBITSATONE = 0x3FFFFFFFFFFFF;

struct NewData {
  NewData(uint64_t d) : data{d & FIFTYBITSATONE} {}
  uint64_t data;
};

struct Never {
};

struct ErrorFound {
  ErrorFound(const std::string& msg) : message{msg} {}
  std::string message;
};

} // namespace o2::mch::raw
#endif
