#include "MCHDigitOccupancy.h"

/// A simple program to "spy" on MCH digit messages being exchanged
/// between two aliceHLTwrappers
/// \author Laurent Aphecetche (laurent.aphecetche at cern.ch)

#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <iterator>

namespace po = boost::program_options;
using namespace std;

int main(int argc, char* argv[])
{
  string source;
  bool interactive(false);
  string mapping;

  try {

    po::options_description desc("Usage");
    desc.add_options()
        ("help,h", "produces this usage message")
        ("source,s", po::value<string>(&source), "address to get the messages from")
        ("interactive,x", "interactive loop")
        ("mapping,s", po::value<string>(&mapping), "compact mapping binary file")
        ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if ( vm.count("interactive") )
    {
        interactive=true;
    }

    if (vm.count("help")) {
      cout << desc << endl;
      return 1;
    }

    if (vm.count("source")) {
      cout << vm["source"].as<string>() << endl;
    }
    else {
      cerr << "Source option not given." << endl;
      return 2;
    }

    if (vm.count("mapping"))
    {
        cout << vm["mapping"].as<string>() << endl;
    }
    else
    {
        cerr << "Mapping file not given." << endl;
        return 3;
    }
  } catch (exception& e) {
    cerr << "error: " << e.what() << "\n";
    return 1;
  } catch (...) {
    cerr << "Exception of unknown type!\n";
  }

  std::ifstream input(mapping.c_str(),std::ios::binary);

  int nmanus;

  input.read((char*)&nmanus,sizeof(int));
 
  assert(nmanus==16828);

  std::vector<uint32_t> manus;
  std::vector<uint8_t> npad;

  manus.resize(nmanus,0);
  npad.resize(nmanus,0);

  input.read((char*)&manus[0],nmanus*sizeof(uint32_t));
  input.read((char*)&npad[0],nmanus*sizeof(uint8_t));

  int ix(0);
  for ( auto m : manus)
  {
    std::cout << ix << " " << m << " " << (m & 0x00000FFF) <<  " " << ( ( m & 0x00FFF000 ) >> 12 ) << " : " << (int)npad[ix] << std::endl;
    ++ix;
  }

  AliceO2::MUON::MCHDigitOccupancy occupancy(manus,npad);

  FairMQChannel inputChannel("pull","connect",source);

  occupancy.SetTransport("zeromq");

  occupancy.fChannels["data-in"].push_back(inputChannel);

  occupancy.ChangeState(FairMQStateMachine::INIT_DEVICE);
  occupancy.WaitForEndOfState(FairMQStateMachine::INIT_DEVICE);

  occupancy.ChangeState(FairMQStateMachine::INIT_TASK);
  occupancy.WaitForEndOfState(FairMQStateMachine::INIT_TASK);

  if (interactive)
  {
    occupancy.InteractiveStateLoop();
  }
  else
  {
      occupancy.ChangeState(FairMQStateMachine::RUN);
      occupancy.WaitForEndOfState(FairMQStateMachine::RUN);
  }
  return 0;
}
