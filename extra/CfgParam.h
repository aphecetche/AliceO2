// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_COMMON_UTILS_CFG_PARAM_H
#define O2_COMMON_UTILS_CFG_PARAM_H

#include <string_view>

namespace o2::conf
{

class CfgParam
{
 public:
  template <typename T>
  static T getValueAs(std::string_view key)
  {
    return getValueAs(sRegistry, key);
  }

  template <typename T>
  static void setValue(std::string_view mainkey, std::string_view subkey, T x);
};

} // namespace o2::conf
#endif
