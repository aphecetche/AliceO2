// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#define BOOST_TEST_MODULE Test MCHRaw BareElinkEncoder
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <iostream>
#include "BareElinkEncoder.h"
#include "MCHRawCommon/SampaHeader.h"
#include <fstream>
#include <fmt/printf.h>

using namespace o2::mch::raw;

BareElinkEncoder createBareElinkEncoder10()
{
  uint8_t cruId{0};
  uint8_t linkId{0};
  int phase{0};
  bool clusterSumMode{false};

  BareElinkEncoder enc(cruId, linkId, phase, clusterSumMode);

  enc.addChannelData(1, {SampaCluster{20, std::vector<uint16_t>{20}}});
  enc.addChannelData(5, {SampaCluster{100, std::vector<uint16_t>{100, 101}}});
  enc.addChannelData(13, {SampaCluster{260, std::vector<uint16_t>{260, 261, 262}}});
  enc.addChannelData(31, {SampaCluster{620, std::vector<uint16_t>{620, 621, 622, 623}}});

  return enc;
}

BareElinkEncoder createBareElinkEncoder20()
{
  uint8_t cruId{0};
  uint8_t linkId{0};
  int phase{0};
  bool clusterSumMode{true};

  BareElinkEncoder enc(cruId, linkId, phase, clusterSumMode);

  enc.addChannelData(1, {SampaCluster{20, 101}});
  enc.addChannelData(5, {SampaCluster{100, 505}});
  enc.addChannelData(13, {SampaCluster{260, 1313}});
  enc.addChannelData(31, {SampaCluster{620, 3131}});

  return enc;
}

BOOST_AUTO_TEST_SUITE(o2_mch_raw)

BOOST_AUTO_TEST_SUITE(elinkencoder)

BOOST_AUTO_TEST_CASE(CtorBuildsAnEmptyBitSet)
{
  BareElinkEncoder enc(0, 0);
  BOOST_CHECK_EQUAL(enc.len(), 0);
}

BOOST_AUTO_TEST_CASE(AddSingleHitShouldIncreaseSizeBy140Bits)
{
  BareElinkEncoder enc(0, 0);
  auto initialSize = enc.len();
  std::vector<SampaCluster> data = {SampaCluster(20, 10)};
  enc.addChannelData(31, data);
  int expectedSize = initialSize + 100 + 40;
  BOOST_CHECK_EQUAL(enc.len(), expectedSize);
}

BOOST_AUTO_TEST_CASE(AddMultipleHitsShouldIncreateSizeBy140BitsTimeN)
{
  BareElinkEncoder enc(0, 0);
  auto initialSize = enc.len();
  uint8_t chId{31};

  std::vector<SampaCluster> data = {
    SampaCluster(10, 1000),
    SampaCluster(20, 2000),
    SampaCluster(30, 3000),
  };

  enc.addChannelData(chId, data);

  int expectedSize = initialSize + 100 + 40 * data.size();
  BOOST_CHECK_EQUAL(enc.len(), expectedSize);
}

BOOST_AUTO_TEST_CASE(OneChipChargeSumOneCluster)
{
  BareElinkEncoder enc(0, 9, 20);
  auto initialSize = enc.len();
  enc.addChannelData(1, {SampaCluster(20, 101)});
  enc.addChannelData(5, {SampaCluster(100, 505)});
  enc.addChannelData(13, {SampaCluster(260, 1313)});
  enc.addChannelData(31, {SampaCluster(620, 3131)});
  BOOST_CHECK_EQUAL(enc.len(), initialSize + 50 + 4 * 90);
}

BOOST_AUTO_TEST_CASE(OneChipSamplesOneCluster)
{
  uint8_t cruId{0};
  uint8_t linkId{0};
  int phase{0};
  bool clusterSumMode{false};
  BareElinkEncoder enc(cruId, linkId, phase, clusterSumMode);
  auto initialSize = enc.len();
  enc.addChannelData(1, {SampaCluster(20, std::vector<uint16_t>{1, 10, 100, 10, 1})});
  enc.addChannelData(5, {SampaCluster(100, std::vector<uint16_t>{5, 50, 5})});
  enc.addChannelData(13, {SampaCluster(260, std::vector<uint16_t>{
                                              13, 14, 15, 15, 13})});
  enc.addChannelData(31, {SampaCluster(620, std::vector<uint16_t>{31})});
  BOOST_CHECK_EQUAL(enc.len(), initialSize + 50 + 4 * (50 + 20) + 14 * 10);
}

void print(const char* msg, const BareElinkEncoder& enc)
{
  std::cout << msg << "=";
  for (auto i = 0; i < enc.len(); i++) {
    std::cout << (enc.get(i) ? "1" : "0");
  }
  std::cout << "\n";
}

BOOST_AUTO_TEST_CASE(GetShouldThrowIfBitNumberIsBeyondLen20)
{
  BareElinkEncoder enc = createBareElinkEncoder20();
  std::cout << enc << "\n";
  print("encoder20", enc);
  BOOST_CHECK_THROW(enc.get(enc.len()), std::invalid_argument);
  BOOST_CHECK_NO_THROW(enc.get(enc.len() - 1));
}

BOOST_AUTO_TEST_CASE(GetShouldThrowIfBitNumberIsBeyondLen10)
{
  BareElinkEncoder enc = createBareElinkEncoder10();
  std::cout << enc << "\n";
  print("encoder10", enc);
  BOOST_CHECK_THROW(enc.get(enc.len()), std::invalid_argument);
  BOOST_CHECK_NO_THROW(enc.get(enc.len() - 1));
}

BOOST_AUTO_TEST_CASE(FillWithSync)
{
  BareElinkEncoder enc = createBareElinkEncoder20();
  auto s = enc.len();
  enc.fillWithSync(s + 154);
  BOOST_CHECK_EQUAL(enc.len(), s + 154);
  BOOST_CHECK_EQUAL(enc.range(s, s + 49), sampaSync().uint64());
  BOOST_CHECK_EQUAL(enc.range(s + 50, s + 99), sampaSync().uint64());
  BOOST_CHECK_EQUAL(enc.range(s + 100, s + 149), sampaSync().uint64());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
