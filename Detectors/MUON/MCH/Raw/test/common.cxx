#include "common.h"

using namespace o2::mch::raw;

ElinkEncoder encoderExample1()
{
  ElinkEncoder enc(0, 9);

  enc.addRandomBits(13);
  enc.addSync();
  enc.addChannelChargeSum(1, 20, 101);
  enc.addChannelChargeSum(5, 100, 505);
  enc.addChannelChargeSum(13, 260, 1313);
  enc.addChannelChargeSum(31, 620, 3131);

  return enc;
}
