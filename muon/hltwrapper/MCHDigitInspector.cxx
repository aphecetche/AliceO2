#include "MCHDigitInspector.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>

using namespace std;
using namespace AliceO2::Muon;

namespace {

  int a;
  
  void hexdump(uint8_t* data, uint32_t size)
  {
    uint32_t offset(0);
    uint32_t offset2(0);

    for (uint32_t i = 0; i < size; i += 16) {

      unsigned int a = -1.0f;

      printf("  ");

      for (int b = 0; b < 4; ++b) {
        printf("%02X%02X%02X%02X ", data[offset+3],data[offset+2],data[offset+1],data[offset]);
        offset+=4;
      }

      printf(" ");
      for ( int j = 0; j < 16; ++j ) {
        if ( !isprint(data[offset2])) {
          printf(".");
        }
        else {
          printf("%c",data[offset2]);
        }
        ++offset2;
      }

      assert(offset2==offset);

      printf("  %08X",offset-16);

      printf("\n");
    }
  }


  void DumpMessages(const std::vector<FairMQMessage*>& messages)
  {
    int i = 0;
    for ( auto msg : messages ) {

      cout << "---- Message " << i << " " << msg->GetSize() << endl;
      hexdump(reinterpret_cast<uint8_t*>(msg->GetData()),std::min(msg->GetSize(),(size_t)128));
    }

  }
}

void MCHDigitInspector::Run()
{
  std::cout << "MCHDigitInspector::Run" << std::endl;

  std::unique_ptr<FairMQPoller> poller(fTransportFactory->CreatePoller(fChannels["data-in"]));

  int numInputs = fChannels["data-in"].size();

  while (CheckCurrentState (RUNNING)) {

    poller->Poll(10);

    vector<FairMQMessage*> inputMessages;

    for (int i = 0; i < numInputs; i++) {
      if (poller->CheckInput(i)) {
        do {
          unique_ptr<FairMQMessage> msg(fTransportFactory->CreateMessage());
          if (fChannels.at("data-in").at(i).Receive(msg.get())) {
            inputMessages.push_back(msg.release());
           }
        } while (fChannels.at("data-in").at(i).ExpectsAnotherPart());
      }
    }

    if ( inputMessages.size() ) {
      LOG(INFO) << " |---- received " << inputMessages.size() << " messages";
    }

    DumpMessages(inputMessages);

    for (auto i : inputMessages) {
      delete i;
    }

    if ( inputMessages.size() ) {
      inputMessages.clear();
      static int count(0);
      ++count;
      if ( count == 30) ChangeState("END");
    }
  }

}

void MCHDigitInspector::InitTask()
{
  std::cout << "MCHDigitInspector::InitTask" << std::endl;
}
