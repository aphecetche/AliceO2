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
#include <map>
#include "CommonDataFormat/InteractionRecord.h"
#include "MCHBase/Digit.h"

std::map<o2::InteractionTimeRecord, std::vector<o2::mch::Digit>> generateDigits(
  int nofInteractionsPerTimeFrame,
  bool fixed = false);

#endif
