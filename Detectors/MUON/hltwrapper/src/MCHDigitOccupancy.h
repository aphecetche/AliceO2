#ifndef MUON_HLTWRAPPER_MCHDIGITOCCUPANCY_H_
#define MUON_HLTWRAPPER_MCHDIGITOCCUPANCY_H_

#include "FairMQDevice.h"

namespace AliceO2 {
namespace MUON {

class MCHDigitOccupancy : public FairMQDevice {
public:
  MCHDigitOccupancy() = default;
  virtual ~MCHDigitOccupancy() = default;
  virtual void Run();
};

}
}

#endif
