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
#include "BareElinkEncoder.h"
#include "UserLogicElinkEncoder.h"
#include "CRUEncoderImpl.h"
#include "Headers/RAWDataHeader.h"
#include "MCHRawCommon/DataFormats.h"

namespace o2::mch::raw
{
using namespace o2::mch::raw::dataformat;

template <>
std::unique_ptr<CRUEncoder> createCRUEncoder<Bare, ChargeSumMode>(uint8_t cruId)
{
  return std::make_unique<CRUEncoderImpl<Bare, ChargeSumMode, o2::header::RAWDataHeaderV4>>(cruId);
}

template <>
std::unique_ptr<CRUEncoder> createCRUEncoder<Bare, SampleMode>(uint8_t cruId)
{
  return std::make_unique<CRUEncoderImpl<Bare, SampleMode, o2::header::RAWDataHeaderV4>>(cruId);
}

// template <typename CHARGESUM>
// std::unique_ptr<CRUEncoder> createCRUEncodeNoPhase<Bare, CHARGESUM>(uint8_t cruId)
// {
//   GBTEncoder<ElinkEncoder<Bare, CHARGESUM>>::forceNoPhase = true;
//   return std::make_unique<CRUEncoderImpl<ElinkEncoder<Bare, CHARGESUM>, o2::header::RAWDataHeaderV4>>(cruId);
// }
//
// template <typename CHARGESUM>
// std::unique_ptr<CRUEncoder> createCRUEncoder<UserLogic, CHARGESUM>(uint8_t cruId)
// {
//   return std::make_unique<CRUEncoderImpl<ElinkEncoder<UserLogic, CHARGESUM>, o2::header::RAWDataHeaderV4>>(cruId);
// }
//
} // namespace o2::mch::raw
