// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_NOFBITS_H
#define O2_MCH_RAW_NOFBITS_H

#include <cstdlib>
#include <string_view>

void assertNofBits(std::string_view msg, uint8_t, int n);
void assertNofBits(std::string_view msg, uint16_t, int n);
void assertNofBits(std::string_view msg, uint32_t, int n);
void assertNofBits(std::string_view msg, uint64_t, int n);

int nofBits(uint8_t val);
int nofBits(uint16_t val);
int nofBits(uint32_t val);
int nofBits(uint64_t val);

#endif
