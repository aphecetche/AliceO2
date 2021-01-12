// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHConditions/DCSNamer.h"

#include <array>
#include <fmt/printf.h>
#include <iostream>

namespace
{
std::array<int, 156> detElemIds = {
  100, 101, 102, 103,
  200, 201, 202, 203,
  300, 301, 302, 303,
  400, 401, 402, 403,
  500, 501, 502, 503, 504, 505, 506, 507, 508, 509, 510, 511, 512, 513, 514, 515, 516, 517,
  600, 601, 602, 603, 604, 605, 606, 607, 608, 609, 610, 611, 612, 613, 614, 615, 616, 617,
  700, 701, 702, 703, 704, 705, 706, 707, 708, 709, 710, 711, 712, 713, 714, 715, 716, 717, 718, 719, 720, 721, 722, 723, 724, 725,
  800, 801, 802, 803, 804, 805, 806, 807, 808, 809, 810, 811, 812, 813, 814, 815, 816, 817, 818, 819, 820, 821, 822, 823, 824, 825,
  900, 901, 902, 903, 904, 905, 906, 907, 908, 909, 910, 911, 912, 913, 914, 915, 916, 917, 918, 919, 920, 921, 922, 923, 924, 925,
  1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009, 1010, 1011, 1012, 1013, 1014, 1015, 1016, 1017, 1018, 1019, 1020, 1021, 1022, 1023, 1024, 1025};
bool isQuadrant(int deId)
{
  return deId < 500;
}

bool isSlat(int deId)
{
  return deId >= 500;
}

int nofDetectionElementsInChamber(int chamberId)
{
  // chamberId must be between 4 and 9 (i.e. slats)
  if (chamberId < 6) {
    return 18;
  }
  return 26;
}

} // namespace

namespace o2::mch::dcs
{

std::optional<ID> detElemId2DCS(int deId)
{
  if (std::find(detElemIds.begin(), detElemIds.end(), deId) == detElemIds.end()) {
    return std::nullopt;
  }
  int chamberId = deId / 100 - 1;
  int id = deId - (chamberId + 1) * 100;

  Side side = Side::Left;

  if (isQuadrant(deId)) {
    if (id == 0 || id == 3) {
      side = Side::Right;
    } else {
      side = Side::Left;
    }
  } else {
    int nofDe = nofDetectionElementsInChamber(chamberId);
    int quarter = nofDe / 4;
    int half = nofDe / 2;
    int threeQuarter = quarter + half;
    if (id <= quarter) {
      id += quarter + 1;
      side = Side::Right;
    } else if (id <= threeQuarter) {
      id = (threeQuarter - id + 1);
      side = Side::Left;
    } else {
      id -= threeQuarter;
      side = Side::Right;
    }
    // dcs convention change : numbering from top, not from bottom
    id = half - id;
  }

  return std::optional<ID>{{id, side, chamberId}};
}

std::string basePattern(ID id)
{
  std::string side = id.side == Side::Left ? "Left" : "Right";
  return "MchHvLv" + side + "/Chamber%02d" + side + "/";
}

std::string quadrantHV(int deId, ID id, int sector)
{
  const auto pattern = basePattern(id) + "Quad%dSect%d";
  return fmt::sprintf(pattern, id.chamberId, id.number, sector);
}

std::string slatHV(int deId, ID id)
{
  const auto pattern = basePattern(id) + "Slat%02d";
  return fmt::sprintf(pattern, id.chamberId, id.number);
}

std::string hvPattern(int deId, int sector = -1)
{
  auto id = detElemId2DCS(deId);
  if (!id.has_value()) {
    return "INVALID";
  }
  std::string base;
  if (isQuadrant(deId)) {
    base = quadrantHV(deId, id.value(), sector);
  } else {
    base = slatHV(deId, id.value());
  }
  return base + ".actual.%s";
}

std::string measurementName(MeasurementType m)
{
  switch (m) {
    case MeasurementType::Voltage:
      return "vMon";
    case MeasurementType::Current:
      return "iMon";
    case MeasurementType::Analog:
      return "an";
    case MeasurementType::Digital:
      return "di";
  }
}

std::vector<std::string> measurement(const std::vector<std::string>& patterns,
                                     MeasurementType m)
{
  std::vector<std::string> result;

  result.resize(patterns.size());

  std::transform(patterns.begin(), patterns.end(), result.begin(), [&m](std::string s) {
    return fmt::sprintf(s, measurementName(m));
  });

  std::sort(result.begin(), result.end());
  return result;
}

/// Generate aliases of the 188 HV DCS channels
///
/// St 1 ch  1 : 12 channels
///      ch  2 : 12 channels
/// St 2 ch  3 : 12 channels
///      ch  4 : 12 channels
/// St 3 ch  5 : 18 channels
///      ch  6 : 18 channels
/// St 4 ch  7 : 26 channels
///      ch  8 : 26 channels
/// St 5 ch  9 : 26 channels
///      ch 10 : 26 channels
///
std::vector<std::string> aliasesForHV(std::vector<MeasurementType> types)
{
  std::vector<std::string> patterns;

  for (auto deId : detElemIds) {
    if (isQuadrant(deId)) {
      for (auto sector = 0; sector < 3; ++sector) {
        patterns.emplace_back(hvPattern(deId, sector));
      }
    } else {
      patterns.emplace_back(hvPattern(deId));
    }
  }

  std::vector<std::string> aliases;

  for (auto m : types) {
    if (m == MeasurementType::Voltage ||
        m == MeasurementType::Current) {
      auto aliasPerType = measurement(patterns, m);
      aliases.insert(aliases.begin(), aliasPerType.begin(), aliasPerType.end());
    }
  }

//  std::sort(aliases.begin(),aliases.end());
  // std::cout << "aliasesForHV\n";
  // for (auto a : aliases) {
  //   std::cout << a << "\n";
  // }
  return aliases;
}

/// Generate XXX LV groups (108 per voltage x 2 voltages for Dual Sampa
/// front end card + YY for readout SOLAR cards)
///
/// St 1 ch  1 left or right : 4 groups
///      ch  2 left or right : 4 groups
/// St 2 ch  3 left or right : 4 groups
///      ch  4 left or right : 4 groups
/// St 3 ch  5 left or right : 5 groups
///      ch  6 left or right : 5 groups
/// St 4 ch  7 left or right : 7 groups
///      ch  8 left or right : 7 groups
/// St 5 ch  9 left or right : 7 groups
///      ch 10 left or right : 7 groups
///
std::vector<std::string> aliasesForLV(std::vector<MeasurementType> types)
{
  return {};
}

/// Generate DCS alias names, for MUON Tracker High and Low Voltage systems
std::vector<std::string> aliases(std::vector<MeasurementType> types)
{
  auto hv = aliasesForHV(types);
  auto lv = aliasesForLV(types);

  auto aliases = hv;
  aliases.insert(aliases.begin(), lv.begin(), lv.end());

  return aliases;
}

} // namespace o2::mch::dcs

//
// TObjArray* aliases = new TObjArray;
// aliases->SetOwner(kTRUE);
//
// Int_t nMeasures = (fDetector == kTriggerDet) ? kNDCSMeas : 1;
//
// for(Int_t iMeas=0; iMeas<nMeasures; iMeas++){
//
//   AliMpDEIterator it;
//
//   it.First();
//
//   Int_t voltageType[] = { -1,0,1 };
//
//   while (!it.IsDone())
//   {
//     Int_t detElemId = it.CurrentdeId();
//
//     switch (fDetector){
//       case kTrackerDet:
//       {
//         switch ( AliMpDEManager::GetStationType(detElemId) )
//         {
//           case AliMp::kStation12:
//           {
//             for ( int sector = 0; sector < 3; ++sector)
//             {
//               // HV voltage
//               aliases->Add(new TObjString(DCSAliasName(detElemId,sector)));
//               // HV current
//               aliases->Add(new TObjString(DCSAliasName(detElemId,sector,AliMpDCSNamer::kDCSI)));
//             }
//
//             AliMp::PlaneType planeType[] = { AliMp::kBendingPlane, AliMp::kNonBendingPlane };
//
//             // LV groups, one per voltage (analog negative, digital, analog positive)
//             // per plane (bending / non-bending)
//             for ( int pt = 0; pt < 2; ++pt )
//             {
//               for ( int i = 0; i < 3; ++i )
//               {
//                 TString name = DCSMCHLVAliasName(detElemId,voltageType[i],planeType[pt]);
//                 aliases->Add(new TObjString(name));
//               }
//             }
//           }
//           break;
//           case AliMp::kStation345:
//           {
//             // HV voltage
//             aliases->Add(new TObjString(DCSAliasName(detElemId)));
//             // HV current
//             aliases->Add(new TObjString(DCSAliasName(detElemId,0,AliMpDCSNamer::kDCSI)));
//             // HV switches
//             for ( Int_t i = 0; i < NumberOfPCBs(detElemId); ++i )
//             {
//               aliases->Add(new TObjString(DCSSwitchAliasName(detElemId,i)));
//             }
//             // LV groups, one per voltage (analog negative, digital, analog positive)
//             for ( int i = 0; i < 3; ++i )
//             {
//               TString name = DCSMCHLVAliasName(detElemId,voltageType[i]);
//               // for Station345 some detection elements share the same voltage group,
//               // so we must insure we're not adding the same one several times
//               if (!aliases->FindObject(name))
//               {
//                 aliases->Add(new TObjString(name));
//               }
//             }
//           }
//           break;
//           default:
//           break;
//         }
//       }
//         break;
//       case kTriggerDet:
//       {
//         switch ( AliMpDEManager::GetStationType(detElemId) )
//         {
//           case AliMp::kStationTrigger:
//             AliDebug(10,Form("Current DetElemId %i",detElemId));
//             aliases->Add(new TObjString(DCSAliasName(detElemId,0,iMeas)));
//             break;
//           default:
//             break;
//         }
//       }
//         break;
//     }
//     it.Next();
//   } // loop on detElemId
// } // Loop on measurement type
//
// if (pattern && strlen(pattern)>0)
// {
//   // remove aliases not containing the input pattern
//   TObjArray* tmp = new TObjArray;
//   tmp->SetOwner(kTRUE);
//   for ( Int_t i = 0; i <= aliases->GetLast(); ++i )
//   {
//      TString name = static_cast<TObjString*>(aliases->At(i))->String();
//      if (name.Contains(pattern))
//      {
//        tmp->Add(new TObjString(name.Data()));
//      }
//   }
//   delete aliases;
//   aliases = tmp;
// }
// return aliases;
