// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "CompactBitSetString.h"
#include <string>
#include <fmt/format.h>
#include "MCHRaw/SampaHeader.h"

namespace o2::mch::raw
{

std::string compactString(const BitSet& bs)
{
  // replaces multiple sync patterns by nxSYNC
  static BitSet syncWord(sampaSync().uint64(), 50);

  if (bs.size() < 49) {
    return bs.stringLSBLeft();
  }
  std::string s;

  int i = 0;
  int nsync = 0;
  while (i + 49 < bs.len()) {
    bool sync{false};
    while (i + 49 < bs.len() && bs.subset(i, i + 49) == syncWord) {
      i += 50;
      nsync++;
      sync = true;
    }
    if (sync) {
      s += fmt::format("[{}SYNC]", nsync);
    } else {
      nsync = 0;
      s += bs.get(i) ? "1" : "0";
      i++;
    }
  }
  for (int j = i; j < bs.len(); j++) {
    s += bs.get(j) ? "1" : "0";
  }
  return s;
}
} // namespace o2::mch::raw
