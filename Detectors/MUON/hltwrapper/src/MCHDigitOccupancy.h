#ifndef MUON_HLTWRAPPER_MCHDIGITOCCUPANCY_H_
#define MUON_HLTWRAPPER_MCHDIGITOCCUPANCY_H_

#include "FairMQDevice.h"
#include "FairMQMessage.h"
#include <vector>
#include <map>

namespace AliceO2 {
namespace MUON {

class MCHDigitOccupancy : public FairMQDevice {
public:
  MCHDigitOccupancy(const std::vector<uint32_t>& manuAbsId,
          const std::vector<uint8_t>& npad);
  virtual ~MCHDigitOccupancy() = default;

protected:

  bool HandleData(FairMQMessagePtr& input, int);
  virtual void ResetTask();

private:
  void ComputeOccupancy();
  void DumpOccupancy(float threshold=0.03); // 3 %

private:
  std::vector<uint32_t> mManuAbsId;
  std::vector<uint8_t> mNumberOfPadsInManu;
  std::map<uint32_t,int> mManuAbsIdToIndex;
  std::vector<float> mManuOccupancy;
  std::vector<uint64_t> mManuHit;
  uint32_t mNumberOfReceivedEvents;
  uint32_t mNumberOfRequiredEventsForComputation=1000;
};
}
}

#endif
