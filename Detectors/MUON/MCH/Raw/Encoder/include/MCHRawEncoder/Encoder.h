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
#include <set>

namespace o2::mch::raw
{

/// createCRUEncoder creates an encoder
///
/// template parameters :
///
/// \param FORMAT defines the data format (either BareFormat or UserLogic)
/// \param CHARGESUM defines the data format mode (either SampleMode or ChargeSumMode)
/// \param RDH defines the RawDataHeader to be used (v4, v5, etc...)
/// \param forceNoPhase to be deprecated ?
///
/// function parameters :
/// \param cruId : a (valid) identifier for a CRU
/// \param solarIds : the set of identifiers of the Solar cards for this CRU.
template <typename FORMAT, typename CHARGESUM, typename RDH, bool forceNoPhase = false>
std::unique_ptr<CRUEncoder> createCRUEncoder(uint8_t cruId, std::set<uint16_t> solarIds);

} // namespace o2::mch::raw
#endif
