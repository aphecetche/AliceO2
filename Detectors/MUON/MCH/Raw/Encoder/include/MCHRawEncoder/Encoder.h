// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_ENCODER_H
#define O2_MCH_RAW_ENCODER_H

#include "MCHRawEncoder/CRUEncoder.h"
#include <memory>

namespace o2::mch::raw
{

std::unique_ptr<CRUEncoder> createBareCRUEncoder(uint8_t cruId, bool chargeSum = true);

std::unique_ptr<CRUEncoder> createBareCRUEncoderNoPhase(uint8_t cruId, bool chargeSum = true);

std::unique_ptr<CRUEncoder> createUserLogicCRUEncoder(uint8_t cruId, bool chargeSum = true);

}; // namespace o2::mch::raw

#endif
