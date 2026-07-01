#include <iostream>
#include "DeviceManager.hpp"

using std::cout;
using std::endl;

int main() {

  DeviceManager dm;

  cout << "MotionPlusPlus running..." << endl;

  auto input_devices = dm.scan();

  auto res = dm.populateMetadata(input_devices);

  if (!res) {
    std::cerr << "Error while populating metadata." << std::endl;
    return 1;
  }

  for (auto &in_d : input_devices) {
    cout << in_d.name << " - " << in_d.vendor << " - " << in_d.product << " - " << in_d.bus << " - " << in_d.phys << " - " << in_d.uniq << endl;
  }

  return 0;
}
