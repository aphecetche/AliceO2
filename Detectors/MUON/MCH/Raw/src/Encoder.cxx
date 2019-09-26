// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHRaw/Encoder.h"
#include "MCHRaw/SampaHeader.h"

namespace
{

int computeHamming(uint64_t header)
{
  return 0;
}
} // namespace

namespace o2::mch::raw
{

BitSet Encoder::oneDS(int dsid, int timestamp, gsl::span<int> channels, gsl::span<int> adcs)
{
  return BitSet::from(sampaSync().asUint64());
}

} // namespace o2::mch::raw
