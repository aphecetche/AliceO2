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
#include "MCHRawElecMap/Mapper.h"
#include "MCHMappingInterface/CathodeSegmentation.h"
#include "MCHContour/SVGWriter.h"
#include "MCHMappingSegContour/SegmentationContours.h"
#include "MCHMappingSegContour/CathodeSegmentationContours.h"
#include "MCHMappingSegContour/CathodeSegmentationSVGWriter.h"

using namespace rapidjson;
using namespace o2::mch::mapping;
using namespace o2::mch::raw;
using namespace o2::mch::contour;
using namespace std;

namespace po = boost::program_options;

// return the color corresponding to value (which is expected between 0 and 1)
// palette taken from http://colorbrewer2.org/#type=sequential&scheme=YlGnBu&n=8
std::string getColor(float value)
{
  static const std::array<std::string, 8> colors = {"#ffffd9", "#edf8b1", "#c7e9b4", "#7fcdbb", "#41b6c4", "#1d91c0", "#225ea8", "#0c2c84"};

  if (value < 0 || value > 1.0) {
    return "black";
  }
  return colors[static_cast<int>(std::floor(value * 8))];
}

struct PadInfo {
  int deId;
  int index;
  double value;
  double x;
  double y;
  double dx;
  double dy;
  bool bending;
};

std::vector<PadInfo> filter(const Value& channels,
                            std::function<std::optional<PadInfo>(const Value& v)> selector)
{
  std::vector<PadInfo> pads;

  for (const auto& c : channels.GetArray()) {
    auto s = selector(c);
    if (s.has_value()) {
      pads.emplace_back(s.value());
    }
  }
  return pads;
}

std::pair<double, double> getRange(gsl::span<PadInfo> pads)
{
  double minValue = std::numeric_limits<double>::max();
  double maxValue = std::numeric_limits<double>::min();

  for (auto p : pads) {
    minValue = std::min(minValue, p.value);
    maxValue = std::max(maxValue, p.value);
  }
  return std::make_pair(minValue, maxValue);
}

std::function<std::optional<DsDetId>(const Value& v)> getDsDetId()
{
  auto e2d = o2::mch::raw::createElec2DetMapper<o2::mch::raw::ElectronicMapperGenerated>(o2::mch::raw::deIdsOfCH6R); // FIXME: use them all
  return [e2d](const Value& v) -> std::optional<DsDetId> {
    const auto& o = v.GetObject();
    auto sid = o["id"].GetString();
    auto dsElecId = o2::mch::raw::decodeDsElecId(sid);
    if (!dsElecId.has_value()) {
      std::cout << fmt::format("{} is not a valid dsElecId\n", sid);
      return std::nullopt;
    }
    auto dsDetId = e2d(dsElecId.value());
    if (!dsDetId.has_value()) {
      std::cout << fmt::format("Got no dsDetId for dsElecId={}\n", sid);
      return std::nullopt;
    }
    return dsDetId;
  };
}

std::set<int> getDeIds(const Value& channels)
{
  auto toDsDetId = getDsDetId();
  std::set<int> deids;
  for (const auto& c : channels.GetArray()) {
    auto dsDetId = toDsDetId(c);
    if (dsDetId.has_value()) {
      deids.insert(dsDetId->deId());
    }
  }
  return deids;
}

std::function<std::optional<PadInfo>(const Value& v)> getPadInfoCreator(int deId, std::string what)
{
  o2::mch::mapping::Segmentation seg{deId};
  auto toDsDetId = getDsDetId();

  return [seg, toDsDetId, deId, what](const Value& v) -> std::optional<PadInfo> {
    auto dsDetId = toDsDetId(v);
    if (!dsDetId.has_value()) {
      return std::nullopt;
    }
    if (dsDetId.value().deId() != deId) {
      return std::nullopt;
    }
    const auto& o = v.GetObject();
    auto chId = o2::mch::raw::decodeChannelId(o["id"].GetString());
    if (!chId.has_value()) {
      return std::nullopt;
    }
    auto padIndex = seg.findPadByFEE(dsDetId->dsId(), chId.value());
    if (!seg.isValid(padIndex)) {
      return std::nullopt;
    }
    return PadInfo{
      deId, padIndex, o[what.c_str()].GetDouble(),
      seg.padPositionX(padIndex),
      seg.padPositionY(padIndex),
      seg.padSizeX(padIndex) / 2.0,
      seg.padSizeY(padIndex) / 2.0,
      seg.isBendingPad(padIndex)};
  };
}

std::function<std::optional<PadInfo>(const Value& v)>
  selectByDeId(int deId, std::string what)
{
  auto toPadInfo = getPadInfoCreator(deId, what);

  return [deId, toPadInfo](const Value& v) -> std::optional<PadInfo> {
    auto pad = toPadInfo(v);
    if (!pad.has_value()) {
      return std::nullopt;
    }
    if (pad->deId != deId) {
      return std::nullopt;
    }
    return pad;
  };
}

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

  IStreamWrapper isw(std::cin);
  Document d;
  d.ParseStream(isw);

  std::map<int, o2::mch::contour::SVGWriter*> svgWriters;

  const Value& channels = d["channels"];

  auto deids = getDeIds(channels);

  for (auto deId : deids) {

    auto pads = filter(channels, selectByDeId(deId, "noise"));

    std::array<CathodeSegmentation, 2> cathodes{
      CathodeSegmentation{deId, true},
      CathodeSegmentation{deId, false}};

    std::array<o2::mch::contour::SVGWriter, 2> writers{
      SVGWriter(getBBox(cathodes[0])),
      SVGWriter(getBBox(cathodes[1]))};

    for (auto& w : writers) {
      w.svgGroupStart("pads");
    }

    std::pair<double, double> range = getRange(pads);

    for (const auto& p : pads) {
      writers[p.bending == true ? 0 : 1].polygon(o2::mch::contour::Polygon<double>{
                                                   {p.x - p.dx, p.y - p.dy}, {p.x + p.dx, p.y - p.dy}, {p.x + p.dx, p.y + p.dy}, {p.x - p.dx, p.y + p.dy}, {p.x - p.dx, p.y - p.dy}},
                                                 getColor(p.value / range.second));
    }

    // rapidjson::OStreamWrapper osw(std::cout);
    // rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
    // d.Accept(writer);

    std::cout << "<html>\n";
    std::cout << "<style>\n";
    std::cout << R"(
  .detectionelements{
    stroke: black;
    stroke-width: 0.1px;
    fill:none;
  }
  svg {
  border: 1px solid blue;
  padding: 10px;
  }
  )";
    std::cout << "</style>\n";
    std::cout << "<body>\n";

    for (auto bending : {0, 1}) {
      auto& w = writers[bending];
      w.svgGroupEnd();
      w.svgGroupStart("detectionelements");
      w.contour(getEnvelop(cathodes[bending]));
      w.svgGroupEnd();
      w.writeSVG(std::cout);
    }
    std::cout << "</body>\n";
    std::cout << "</html>\n";
  }

  return 0;
}
