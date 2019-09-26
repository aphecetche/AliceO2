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

#include "BitSet.h"
#include <gsl/span>

namespace o2
{
namespace mch
{
namespace raw
{

class Encoder
{
 public:
  // TODO : remove bitset completely from the interface (might use it in the implementation still)
  // just use it to start
  // should either return a bytes buffer or write into one give bytes buffer
  // (as we know beforehand how many bytes are required) ?
  void appendOneDualSampa(BitSet& bs, int dsid, int timestamp, gsl::span<int> channels, gsl::span<int> adcs);
};
} // namespace raw
} // namespace mch
} // namespace o2
#endif
