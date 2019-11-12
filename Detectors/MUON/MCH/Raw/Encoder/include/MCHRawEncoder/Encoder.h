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

template <typename FORMAT, typename CHARGESUM>
std::unique_ptr<CRUEncoder> createCRUEncoder(uint8_t cruId);

template <typename FORMAT, typename CHARGESUM>
std::unique_ptr<CRUEncoder> createCRUEncoderNoPhase(uint8_t cruId);

} // namespace o2::mch::raw
#endif
