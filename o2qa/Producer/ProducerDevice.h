#pragma once

#include <string>
#include <TH1F.h>
#include <FairMQDevice.h>
#include <memory>

#include "Producer.h"

class ProducerDevice : public FairMQDevice
{
public:
  ProducerDevice(std::string producerId,
    			       std::string namePrefix,
    			       std::string title,
    			       float xLow,
    			       float xUp,
    			       int numIoThreads);
  virtual ~ProducerDevice() = default;

  void executeRunLoop();
  void establishChannel(std::string type, std::string method, std::string address, std::string channelName);

protected:
  ProducerDevice() = default;
  virtual void Run();

private:
  std::shared_ptr<Producer> mProducer;
};
