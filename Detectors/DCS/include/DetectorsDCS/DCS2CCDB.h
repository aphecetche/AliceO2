// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_DCS_DCS_2_CCDB_H
#define O2_DCS_DCS_2_CCDB_H

#include "CCDB/CcdbApi.h"
#include "CCDB/CcdbObjectInfo.h"
#include "CommonUtils/MemFileHelper.h"
#include "DetectorsCalibration/Utils.h"
#include "Framework/DataAllocator.h"
#include <map>
#include <string>

namespace o2
{
namespace dcs
{

using TFType = uint64_t;

template <typename T>
void prepareCCDBobject(T& obj, o2::ccdb::CcdbObjectInfo& info, const std::string& path, TFType tf,
                       const std::map<std::string, std::string>& md)
{

  // prepare all info to be sent to CCDB for object obj
  auto clName = o2::utils::MemFileHelper::getClassName(obj);
  auto flName = o2::ccdb::CcdbApi::generateFileName(clName);
  info.setPath(path);
  info.setObjectType(clName);
  info.setFileName(flName);
  info.setStartValidityTimestamp(tf);
  info.setEndValidityTimestamp(99999999999999);
  info.setMetaData(md);
}

template <typename T>
void sendOutput(const T& obj, o2::ccdb::CcdbObjectInfo& info, framework::DataAllocator& output)
{
  // extract CCDB infos and calibration objects, convert it to TMemFile and send them to the output
  // copied from LHCClockCalibratorSpec.cxx
  const auto& payload = obj;
  auto image = o2::ccdb::CcdbApi::createObjectImage(&payload, &info);
  LOG(INFO) << "Sending object " << info.getPath() << "/" << info.getFileName() << " of size " << image->size()
            << " bytes, valid for " << info.getStartValidityTimestamp() << " : " << info.getEndValidityTimestamp();

  output.snapshot(framework::Output{o2::calibration::Utils::gDataOriginCLB,
                                    o2::calibration::Utils::gDataDescriptionCLBPayload, 0},
                  *image.get());

  output.snapshot(framework::Output{o2::calibration::Utils::gDataOriginCLB,
                                    o2::calibration::Utils::gDataDescriptionCLBInfo, 0},
                  info);
}

std::vector<framework::OutputSpec> CCDBOutputs() {
  std::vector<framework::OutputSpec> outputs;
  outputs.emplace_back(framework::ConcreteDataTypeMatcher{o2::calibration::Utils::gDataOriginCLB,
                                               o2::calibration::Utils::gDataDescriptionCLBPayload});
  outputs.emplace_back(framework::ConcreteDataTypeMatcher{o2::calibration::Utils::gDataOriginCLB,
                                               o2::calibration::Utils::gDataDescriptionCLBInfo});
  return outputs;
}
} // namespace dcs
} // namespace o2
#endif
