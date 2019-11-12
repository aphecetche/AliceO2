// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_TEST_REF_BUFFERS_H
#define O2_MCH_RAW_TEST_REF_BUFFERS_H

#include <array>
#include <cstdlib>

template <bool CHARGESUM>
extern std::array<uint8_t, 2560> REF_BUFFER_GBT_BARE();

template <bool CHARGESUM>
extern std::array<uint8_t, 96> REF_BUFFER_GBT_USER_LOGIC();

extern std::array<uint8_t, 8336> REF_BUFFER_CRU_BARE;

#endif
