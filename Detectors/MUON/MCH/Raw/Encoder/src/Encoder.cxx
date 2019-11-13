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
#include "Headers/RAWDataHeader.h"
#include "MCHRawCommon/DataFormats.h"
#include "BareElinkEncoder.h"
#include "BareElinkEncoderMerger.h"
#include "UserLogicElinkEncoder.h"
#include "UserLogicElinkEncoderMerger.h"

namespace o2::mch::raw
{
template <>
std::unique_ptr<CRUEncoder> createCRUEncoder<BareFormat, ChargeSumMode>(uint8_t cruId)
{
  return std::make_unique<CRUEncoderImpl<BareFormat, ChargeSumMode, o2::header::RAWDataHeaderV4>>(cruId);
}

template <>
std::unique_ptr<CRUEncoder> createCRUEncoder<BareFormat, SampleMode>(uint8_t cruId)
{
  return std::make_unique<CRUEncoderImpl<BareFormat, SampleMode, o2::header::RAWDataHeaderV4>>(cruId);
}

template <>
std::unique_ptr<CRUEncoder> createCRUEncoderNoPhase<BareFormat, ChargeSumMode>(uint8_t cruId)
{
  GBTEncoder<BareFormat, ChargeSumMode>::forceNoPhase = true;
  return createCRUEncoder<BareFormat, ChargeSumMode>(cruId);
}

template <>
std::unique_ptr<CRUEncoder> createCRUEncoderNoPhase<BareFormat, SampleMode>(uint8_t cruId)
{
  GBTEncoder<BareFormat, SampleMode>::forceNoPhase = true;
  return createCRUEncoder<BareFormat, SampleMode>(cruId);
}

template <>
std::unique_ptr<CRUEncoder> createCRUEncoder<UserLogicFormat, ChargeSumMode>(uint8_t cruId)
{
  return std::make_unique<CRUEncoderImpl<UserLogicFormat, ChargeSumMode, o2::header::RAWDataHeaderV4>>(cruId);
}

template <>
std::unique_ptr<CRUEncoder> createCRUEncoder<UserLogicFormat, SampleMode>(uint8_t cruId)
{
  return std::make_unique<CRUEncoderImpl<UserLogicFormat, SampleMode, o2::header::RAWDataHeaderV4>>(cruId);
}

} // namespace o2::mch::raw
