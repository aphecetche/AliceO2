#include <csignal>
#include <FairMQTransportFactoryZMQ.h>

#include "SystemController.h"

using namespace std;

SystemController systemController("CentralSystemController", "systemController_log.txt", 1);

int main(int argc, char** argv)
{
    LOG(INFO) << "PID: " << getpid();
    LOG(INFO) << "SystemController id: " 
              << systemController.GetProperty(SystemController::Id, "default_id");

    systemController.establishChannel("req", "connect", "tcp://localhost:5001", "data");

    systemController.executeRunLoop();
    
}
