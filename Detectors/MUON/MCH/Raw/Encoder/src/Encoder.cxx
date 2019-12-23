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
#include "EncoderImpl.h"
#include "Headers/RAWDataHeader.h"
#include "MCHRawCommon/DataFormats.h"
#include "BareElinkEncoder.h"
#include "BareElinkEncoderMerger.h"
#include "UserLogicElinkEncoder.h"
#include "UserLogicElinkEncoderMerger.h"
#include "MCHRawElecMap/Solar2CruMapper.h"
#include <gsl/span>

namespace o2::mch::raw
{
namespace impl
{
// cannot partially specialize a function, so create a struct (which can
// be specialized) and use it within the function below.

template <typename FORMAT, typename CHARGESUM, typename RDH, bool forceNoPhase>
struct EncoderCreator {
  static std::unique_ptr<Encoder> _(Solar2CruMapper solar2cru)
  {
    GBTEncoder<FORMAT, CHARGESUM>::forceNoPhase = forceNoPhase;
    return std::make_unique<EncoderImpl<FORMAT, CHARGESUM, RDH>>(solar2cru);
  }
};
} // namespace impl

template <typename FORMAT, typename CHARGESUM, typename RDH, bool forceNoPhase>
std::unique_ptr<Encoder> createEncoder(Solar2CruMapper solar2cru)
{
  return impl::EncoderCreator<FORMAT, CHARGESUM, RDH, forceNoPhase>::_(solar2cru);
}

// define only the specializations we use

template std::unique_ptr<Encoder> createEncoder<BareFormat, SampleMode, o2::header::RAWDataHeaderV4, true>(Solar2CruMapper);
template std::unique_ptr<Encoder> createEncoder<BareFormat, SampleMode, o2::header::RAWDataHeaderV4, false>(Solar2CruMapper);

template std::unique_ptr<Encoder> createEncoder<BareFormat, ChargeSumMode, o2::header::RAWDataHeaderV4, true>(Solar2CruMapper);
template std::unique_ptr<Encoder> createEncoder<BareFormat, ChargeSumMode, o2::header::RAWDataHeaderV4, false>(Solar2CruMapper);

template std::unique_ptr<Encoder> createEncoder<UserLogicFormat, SampleMode, o2::header::RAWDataHeaderV4, true>(Solar2CruMapper);
template std::unique_ptr<Encoder> createEncoder<UserLogicFormat, SampleMode, o2::header::RAWDataHeaderV4, false>(Solar2CruMapper);

template std::unique_ptr<Encoder> createEncoder<UserLogicFormat, ChargeSumMode, o2::header::RAWDataHeaderV4, true>(Solar2CruMapper);
template std::unique_ptr<Encoder> createEncoder<UserLogicFormat, ChargeSumMode, o2::header::RAWDataHeaderV4, false>(Solar2CruMapper);

} // namespace o2::mch::raw
