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

#include "boost/program_options.hpp"
#include <iostream>
#include <fmt/format.h>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include "MCHRawElecMap/DsElecId.h"
#include "MCHRawElecMap/Mapper.h"
#include "MCHRawElecMap/ElectronicMapperGenerated.h"
#include "MCHMappingInterface/CathodeSegmentation.h"
#include "MCHContour/SVGWriter.h"
#include "MCHMappingSegContour/SegmentationContours.h"
#include "MCHMappingSegContour/CathodeSegmentationContours.h"
#include "MCHMappingSegContour/CathodeSegmentationSVGWriter.h"
using namespace rapidjson;
using namespace std;

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
  std::string prefix;
  po::variables_map vm;
  po::options_description generic("Generic options");

  // clang-format off
  generic.add_options()
      ("help,h", "produce help message")
      ("prefix,p",po::value<std::string>(&prefix),"prefix for output file(s)")
      ;
  // clang-format on

  po::options_description cmdline;
  cmdline.add(generic);

  po::store(po::command_line_parser(argc, argv).options(cmdline).run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << generic << "\n";
    return 2;
  }

  auto e2d = o2::mch::raw::createElec2DetMapper<o2::mch::raw::ElectronicMapperGenerated>(o2::mch::raw::deIdsOfCH6R);

  IStreamWrapper isw(std::cin);
  Document d;
  d.ParseStream(isw);

  std::map<int, o2::mch::contour::SVGWriter*> svgWriters;

  const Value& channels = d["channels"];

  //auto deids = getListOfDetectionElements(channels);
  std::vector<int> deids;
  deids.emplace_back(600);
  deids.emplace_back(604);

  for (auto deId : deids) {
    o2::mch::mapping::Segmentation seg{deId};
    o2::mch::mapping::CathodeSegmentation cseg{deId, true};
    auto w = new o2::mch::contour::SVGWriter(o2::mch::mapping::getBBox(cseg));
    w->addStyle(o2::mch::mapping::svgCathodeSegmentationDefaultStyle());
    w->svgGroupStart("detectionelements");
    w->contour(getEnvelop(cseg));
    w->svgGroupEnd();
    w->svgGroupStart("pads");
    svgWriters.emplace(deId, w);

    for (const auto& c : channels.GetArray()) {
      const auto& o = c.GetObject();
      auto dsElecId = o2::mch::raw::decodeDsElecId(o["id"].GetString());
      auto dsDetId = e2d(dsElecId);
      if (!dsDetId.has_value()) {
        continue;
      }
      if (dsDetId.value().deId() != deId) {
        continue;
      }
      auto chId = 0; //decodeChannelId(o["id"].GetString());
      auto padIndex = seg.findPadByFEE(dsDetId.value().dsId(), chId);
      double x = seg.padPositionX(padIndex);
      double y = seg.padPositionY(padIndex);
      double dx = seg.padSizeX(padIndex) / 2.0;
      double dy = seg.padSizeY(padIndex) / 2.0;
      w->polygon(o2::mch::contour::Polygon<double>{
                   {x - dx, y - dy}, {x + dx, y - dy}, {x + dx, y + dy}, {x - dx, y + dy}, {x - dx, y - dy}},
                 "#00FF00");
    }
  }

  // rapidjson::OStreamWrapper osw(std::cout);
  // rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
  // d.Accept(writer);

  for (auto p : svgWriters) {
    p.second->svgGroupEnd();
    std::cout << p.first << "\n";
    p.second->writeHTML(std::cout);
  }
  return 0;
}
