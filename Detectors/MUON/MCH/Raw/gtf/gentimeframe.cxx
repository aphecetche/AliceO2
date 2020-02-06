// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

// gentimeframe
//
// helper program to generate raw data for (the equivalent of) a full
// (sub)timeframe (parameter here is a number of DEs for which we generate
// data)
//
// first generate (or get from somewhere) enough digits to "simulate"
// the occupancy of that (sub)timeframe
//
// then convert all those digits into raw data buffer(s)
// (either bare or userlogic format)
//
// write those buffers to disk
//
// have a look at the output sizes and, more importantly, at how long it takes
// to generate them.
//
// have also a look at the time it takes to read back those raw data files
//
// for the digit generation part, parameters :
//
// - number of events
// - mean number of digits per event (-> Poisson distrib ? simple gauss ?)
//
// - (not needed for this project, but might come handy) : mean time between
// two events (or some kind of event time distribution, to be able to simulate
// e.g. pile-up or ...). That part could take as input some vectors of digits
// and (re-)organize them into time buckets depending on some parameters.
// => use InteractionSampler for that ?
//
//

#include "Headers/RAWDataHeader.h"
#include "MCHBase/Digit.h"
#include "MCHRawCommon/DataFormats.h"
#include "MCHRawCommon/RDHManip.h"
#include "MCHRawElecMap/Mapper.h"
#include "MCHRawEncoder/DigitEncoder.h"
#include <array>
#include "gtfGenerateDigits.h"
#include <boost/program_options.hpp>
#include <fstream>
#include <gsl/span>
#include "Digit2RawConverter.h"

using namespace o2::mch::raw;
namespace po = boost::program_options;

int main(int argc, char* argv[])
{
  po::options_description generic("options");
  bool userLogic{false};
  bool dummyElecMap{false};
  std::string filename;
  po::variables_map vm;
  int nofInteractionsPerTimeFrame{50000};

  // clang-format off
  generic.add_options()
      ("help,h", "produce help message")
      ("userLogic,u",po::bool_switch(&userLogic),"user logic format")
      ("dummyElecMap,d",po::bool_switch(&dummyElecMap),"use a dummy electronic mapping (for testing only)")
      ("outfile,o",po::value<std::string>(&filename)->required(),"output filename")
      ("nintpertf,n",po::value<int>(&nofInteractionsPerTimeFrame),"number of interactions per timeframe")
      ;
  // clang-format on

  po::options_description cmdline;
  cmdline.add(generic);

  po::store(po::command_line_parser(argc, argv).options(cmdline).run(), vm);

  if (vm.count("help")) {
    std::cout << generic << "\n";
    return 2;
  }

  try {
    po::notify(vm);
  } catch (boost::program_options::error& e) {
    std::cout << "Error: " << e.what() << "\n";
    exit(1);
  }

  po::notify(vm);

  std::ofstream outfile(filename);

  std::array<int, 2> deIds = {500, 501};
  auto det2elec = (dummyElecMap ? createDet2ElecMapper<ElectronicMapperDummy>(deIds) : createDet2ElecMapper<ElectronicMapperGenerated>(deIds));

  auto encoder = createDigitEncoder(userLogic, det2elec);

  auto solar2cru = (dummyElecMap ? createSolar2CruLinkMapper<ElectronicMapperDummy>() : createSolar2CruLinkMapper<ElectronicMapperGenerated>());

  auto digits = generateDigits(nofInteractionsPerTimeFrame, true);
  digit2raw<o2::header::RAWDataHeaderV4>(digits, encoder, solar2cru, outfile);

  return 0;
}
