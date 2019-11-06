// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHRawEncoder/Encoder.h"
#include "CRUEncoderImpl.h"
#include "BareGBTEncoder.h"
#include "UserLogicGBTEncoder.h"
#include "Headers/RAWDataHeader.h"

namespace o2::mch::raw
{

std::unique_ptr<CRUEncoder> createBareCRUEncoder(uint8_t cruId, bool chargeSum)
{
  return std::make_unique<CRUEncoderImpl<BareGBTEncoder, o2::header::RAWDataHeaderV4>>(cruId, chargeSum);
}

std::unique_ptr<CRUEncoder> createBareCRUEncoderNoPhase(uint8_t cruId, bool chargeSum)
{
  BareGBTEncoder::forceNoPhase = true;
  return std::make_unique<CRUEncoderImpl<BareGBTEncoder, o2::header::RAWDataHeaderV4>>(cruId, chargeSum);
}

std::unique_ptr<CRUEncoder> createUserLogicCRUEncoder(uint8_t cruId, bool chargeSum)
{
  return std::make_unique<CRUEncoderImpl<UserLogicGBTEncoder, o2::header::RAWDataHeaderV4>>(cruId, chargeSum);
}

} // namespace o2::mch::raw
