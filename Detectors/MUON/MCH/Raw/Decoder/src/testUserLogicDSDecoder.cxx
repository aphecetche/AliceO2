// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

///
/// @author  Laurent Aphecetche

#define BOOST_TEST_MODULE Test MCHRaw UserLogicDSDecoder
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <fstream>
#include <fmt/printf.h>
#include "UserLogicDSDecoder.h"
#include "MCHRawCommon/SampaHeader.h"
using namespace o2::mch::raw;

SampaChannelHandler handlePacketPrint(std::string_view msg)
{
  return [msg](uint8_t cruId, uint8_t linkId, uint8_t chip, uint8_t channel, SampaCluster sc) {
    std::cout << fmt::format("{}chip={:2d} ch={:2d} ", msg, chip, channel);
    std::cout << sc << "\n";
  };
}

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(userlogicdsdecoder)

BOOST_AUTO_TEST_CASE(Test)
{
  UserLogicDSDecoder dec(0, 0, handlePacketPrint("dummy"));
  const uint64_t sync = sampaSync().uint64();
  uint64_t size(5); // cluster size = 5 samples
  uint64_t time(345);
  uint64_t s1{123};
  uint64_t s2{456};
  uint64_t s3{789};
  uint64_t s4{901};
  uint64_t s5{902};
  uint64_t data1 = (size << 40) | (time << 30) | (s1 << 20) | (s2 << 10) | s3;
  uint64_t data2 = (s4 << 40) | (s5 << 30);
  SampaHeader sh;
  sh.nof10BitWords(size + 2); // nof samples + size + time
  sh.packetType(SampaPacketType::Data);
  sh.hammingCode(computeHammingCode(sh.uint64()));
  uint64_t header(sh.uint64());
  std::cout << "Adding sync ...\n";
  dec.append(sync);
  std::cout << "Adding header ...\n";
  dec.append(header);
  std::cout << "Adding first data word ...\n";
  dec.append(data1);
  std::cout << "Adding second data word ...\n";
  dec.append(data2);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
