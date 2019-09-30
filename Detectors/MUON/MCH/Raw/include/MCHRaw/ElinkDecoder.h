// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_ELINK_DECODER_H
#define O2_MCH_RAW_ELINK_DECODER_H

#include "BitSet.h"
#include "SampaHeader.h"
#include <iostream>
#include <functional>

namespace o2
{
namespace mch
{
namespace raw
{

typedef std::function<void(uint8_t, uint8_t, uint16_t, uint32_t)> PacketHandler;

class ElinkDecoder
{
 public:
  ElinkDecoder(int id, PacketHandler packetHandler);
  bool append(bool bit0, bool bit1);
  bool finalize();

 private:
  bool process();
  void clear(int checkpoint);
  void findSync();
  bool getData();
  void getPacket();
  bool append(bool bit);
  friend std::ostream& operator<<(std::ostream& os, const ElinkDecoder& e);

 private:
  int mId;
  int mCheckpoint;
  bool mIsInData;
  int mNofSync;
  BitSet mBitSet;
  SampaHeader mSampaHeader;
  uint64_t mNofBitSeen;
  uint64_t mNofHeaderSeen;
  PacketHandler mPacketHandler;
};

} // namespace raw
} // namespace mch
} // namespace o2

#endif
