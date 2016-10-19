#ifndef MUON_HLTWRAPPER_MCHDIGITINSPECTOR_H_
#define MUON_HLTWRAPPER_MCHDIGITINSPECTOR_H_

/// Simple digit inspector for MCH AliMUONVDigit HLT blocks

#include "FairMQDevice.h"

namespace AliceO2 {
namespace MUON {

class MCHDigitInspector : public FairMQDevice {
public:
  MCHDigitInspector(int maxcount=30) : fCount(0),fMaxCount(maxcount) {}
	virtual ~MCHDigitInspector() = default;

	virtual void Run();
	virtual void InitTask();

private:
    int fCount;
    int fMaxCount;
};

}
}

#endif /* MUON_HLTWRAPPER_MCHDIGITINSPECTOR_H_ */
