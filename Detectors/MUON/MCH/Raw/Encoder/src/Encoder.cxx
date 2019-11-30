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
#include <gsl/span>

namespace o2::mch::raw
{
namespace impl
{
// cannot partially specialize a function, so create a struct (which can
// be specialized) and use it within the function below.

template <typename FORMAT, typename CHARGESUM, typename RDH, bool forceNoPhase>
struct CRUEncoderCreator {
  static std::unique_ptr<CRUEncoder> _(uint8_t cruId, std::set<uint16_t> solarIds)
  {
    GBTEncoder<FORMAT, CHARGESUM>::forceNoPhase = forceNoPhase;
    return std::make_unique<CRUEncoderImpl<FORMAT, CHARGESUM, RDH>>(cruId, solarIds);
  }
};
} // namespace impl

template <typename FORMAT, typename CHARGESUM, typename RDH, bool forceNoPhase>
std::unique_ptr<CRUEncoder> createCRUEncoder(uint8_t cruId, std::set<uint16_t> solarIds)
{
  return impl::CRUEncoderCreator<FORMAT, CHARGESUM, RDH, forceNoPhase>::_(cruId, solarIds);
}

// define only the specializations we use

template std::unique_ptr<CRUEncoder> createCRUEncoder<BareFormat, SampleMode, o2::header::RAWDataHeaderV4, true>(uint8_t, std::set<uint16_t>);
template std::unique_ptr<CRUEncoder> createCRUEncoder<BareFormat, SampleMode, o2::header::RAWDataHeaderV4, false>(uint8_t, std::set<uint16_t>);

template std::unique_ptr<CRUEncoder> createCRUEncoder<BareFormat, ChargeSumMode, o2::header::RAWDataHeaderV4, true>(uint8_t, std::set<uint16_t>);
template std::unique_ptr<CRUEncoder> createCRUEncoder<BareFormat, ChargeSumMode, o2::header::RAWDataHeaderV4, false>(uint8_t, std::set<uint16_t>);

template std::unique_ptr<CRUEncoder> createCRUEncoder<UserLogicFormat, SampleMode, o2::header::RAWDataHeaderV4, true>(uint8_t, std::set<uint16_t>);
template std::unique_ptr<CRUEncoder> createCRUEncoder<UserLogicFormat, SampleMode, o2::header::RAWDataHeaderV4, false>(uint8_t, std::set<uint16_t>);

template std::unique_ptr<CRUEncoder> createCRUEncoder<UserLogicFormat, ChargeSumMode, o2::header::RAWDataHeaderV4, true>(uint8_t, std::set<uint16_t>);
template std::unique_ptr<CRUEncoder> createCRUEncoder<UserLogicFormat, ChargeSumMode, o2::header::RAWDataHeaderV4, false>(uint8_t, std::set<uint16_t>);

} // namespace o2::mch::raw
