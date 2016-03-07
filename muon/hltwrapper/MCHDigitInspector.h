#ifndef MUON_HLTWRAPPER_MCHDIGITINSPECTOR_H_
#define MUON_HLTWRAPPER_MCHDIGITINSPECTOR_H_

/// Simple digit inspector for MCH AliMUONVDigit HLT blocks

#include "FairMQDevice.h"

namespace AliceO2 {
namespace Muon {

class MCHDigitInspector : public FairMQDevice {
public:
  MCHDigitInspector() = default;
	virtual ~MCHDigitInspector() = default;

	virtual void Run();
	virtual void InitTask();
};

}
}

#endif /* MUON_HLTWRAPPER_MCHDIGITINSPECTOR_H_ */
