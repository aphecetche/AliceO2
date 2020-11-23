// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "MCHGeometryTransformer/Transformations.h"
#include "MCHGeometryTransformer/VolumePaths.h"
#include <array>
#include <string>
#include <vector>
#include <TGeoManager.h>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

namespace o2::mch::geo
{

std::array<int, 156> allDeIds = {
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

TransformationCreator transformationFromTGeoManager(const TGeoManager& geo)
{
  return [&geo](int detElemId) -> o2::math_utils::Transform3D {
    if (std::find(begin(allDeIds), end(allDeIds), detElemId) == end(allDeIds)) {
      throw std::runtime_error("Wrong detection element Id");
    }

    std::string volPathName = geo.GetTopVolume()->GetName();

    int nCh = detElemId / 100;

    if (nCh <= 4 && geo.GetVolume("YOUT1")) {
      volPathName += "/YOUT1_1/";
    } else if ((nCh == 5 || nCh == 6) && geo.GetVolume("DDIP")) {
      volPathName += "/DDIP_1/";
    } else if (nCh >= 7 && geo.GetVolume("YOUT2")) {
      volPathName += "/YOUT2_1/";
    } else {
      volPathName += "/";
    }

    volPathName += volumePathName(detElemId);

    TGeoNavigator* navig = gGeoManager->GetCurrentNavigator();

    if (!navig->cd(volPathName.c_str())) {
      throw std::runtime_error("could not get to volPathName=" + volPathName);
    }

    return o2::math_utils::Transform3D{*(navig->GetCurrentMatrix())};
  };
}

TransformationCreator transformationFromJSON(std::istream& in)
{
  rapidjson::IStreamWrapper isw(in);

  rapidjson::Document d;
  d.ParseStream(isw);

  rapidjson::Value& alignables = d["alignables"];
  assert(alignables.IsArray());

  std::map<int, std::array<double, 9>> angles;

  // loop over json document and extract Tait-Bryan angles (yaw,pitch,roll)
  // for each detection element

  return [angles](int detElemId) -> o2::math_utils::Transform3D {
    if (std::find(begin(allDeIds), end(allDeIds), detElemId) == end(allDeIds)) {
      throw std::runtime_error("Wrong detection element Id");
    }
    TGeoHMatrix m;
    // get the angles, convert them to a matrix and build a Transform3D
    // from it
    return o2::math_utils::Transform3D(m);
  };
}

std::array<double, 9> angles2matrix(double yaw, double pitch, double roll)
{
  std::array<double, 9> rot;

  double sinpsi = std::sin(yaw);
  double cospsi = std::cos(yaw);
  double sinthe = std::sin(pitch);
  double costhe = std::cos(pitch);
  double sinphi = std::sin(roll);
  double cosphi = std::cos(roll);
  rot[0] = costhe * cosphi;
  rot[1] = -costhe * sinphi;
  rot[2] = sinthe;
  rot[3] = sinpsi * sinthe * cosphi + cospsi * sinphi;
  rot[4] = -sinpsi * sinthe * sinphi + cospsi * cosphi;
  rot[5] = -costhe * sinpsi;
  rot[6] = -cospsi * sinthe * cosphi + sinpsi * sinphi;
  rot[7] = cospsi * sinthe * sinphi + sinpsi * cosphi;
  rot[8] = costhe * cospsi;
  return rot;
}

std::tuple<double, double, double> matrix2angles(gsl::span<double> rot)
{
  double yaw = std::atan2(-rot[5], rot[8]);
  double pitch = std::asin(rot[2]);
  double roll = std::atan2(-rot[1], rot[0]);
  return std::make_tuple(yaw,
                         pitch,
                         roll);
}

} // namespace o2::mch::geo
