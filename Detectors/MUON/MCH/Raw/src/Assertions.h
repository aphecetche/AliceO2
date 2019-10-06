// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_ASSERTIONS_H
#define O2_MCH_RAW_ASSERTIONS_H

#include <fmt/format.h>

inline void assertIsInRange(std::string what, int value, int min, int max)
{
  if (value < min || value > max) {
    throw std::invalid_argument(fmt::format("{} should be between {} and {} but is {}", what, min, max, value));
  }
}

#endif
