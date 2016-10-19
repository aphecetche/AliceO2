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

  try {

    po::options_description desc("Usage");
    desc.add_options()
        ("help,h", "produces this usage message")
        ("source,s", po::value<string>(&source), "address to get the messages from")
        ("interactive,x", "interactive loop")
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

  } catch (exception& e) {
    cerr << "error: " << e.what() << "\n";
    return 1;
  } catch (...) {
    cerr << "Exception of unknown type!\n";
  }

  AliceO2::MUON::MCHDigitOccupancy occupancy;

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
//
//  occupancy.ChangeState(FairMQStateMachine::END);



  return 0;
}
