// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_EXE_GTF_GENERATE_DIGITS_H
#define O2_MCH_RAW_EXE_GTF_GENERATE_DIGITS_H

#include <vector>
#include <gsl/span>
#include "MCHBase/Digit.h"

std::vector<std::vector<o2::mch::Digit>> generateRandomDigits(int nofEvents, gsl::span<int> deids, float occupancy);

/// generate fake digits
/// each DE gets n digits (where n = (DEID%100)+1), with padids ranging
/// from 0 to n-1
/// each digit has a fixed time of 987 and an adc value = padid*2 << 10 | padid*2
std::vector<std::vector<o2::mch::Digit>> generateFixedDigits(int nofEvents, gsl::span<int> deids);

#endif
