#include "CommonUtils/ConfigurableParam.h"

int main()
{
  o2::conf::ConfigurableParam::printAllKeyValuePairs();

  o2::conf::ConfigurableParam::writeINI(std::cout);

  o2::conf::ConfigurableParam::setValue<bool>("Baz", "mToto", true);
  o2::conf::ConfigurableParam::setValue<bool>("do-not-exist", "mToto", true);

  o2::conf::ConfigurableParam::writeJSON(std::cout);
  o2::conf::ConfigurableParam::writeJSON(std::cout, "do-not-exist");

  return 0;
}
