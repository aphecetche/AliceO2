// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_DATA_FORMATS_H
#define O2_MCH_RAW_DATA_FORMATS_H

#include <cstdint>

namespace o2::mch::raw
{

struct BareFormat {
};

// initial UL format
struct UserLogicFormat {
  union {
    uint64_t word;
    struct {
      uint64_t data : 50;
      uint64_t error : 2;
      uint64_t incomplete : 1;
      uint64_t dsID : 6;
      uint64_t linkID : 5;
    };
  };
};

// version 1 of UL format
// = as initial version with 1 bit less for linkID and one bit more for error
struct UserLogicFormatV1 {
  union {
    uint64_t word;
    struct {
      uint64_t data : 50;
      uint64_t error : 3;
      uint64_t incomplete : 1;
      uint64_t dsID : 6;
      uint64_t linkID : 4;
    };
  };
};

struct ChargeSumMode {
  bool operator()() const { return true; }
};

struct SampleMode {
  bool operator()() const { return false; }
};

using uint10_t = uint16_t;
using uint20_t = uint32_t;
using uint50_t = uint64_t;
}; // namespace o2::mch::raw

#endif
