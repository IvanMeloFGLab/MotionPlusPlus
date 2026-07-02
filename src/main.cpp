#include <iostream>
#include <print>
#include "DeviceManager.hpp"
#include "DeviceConnection.hpp"

using std::println;

int main() {

  DeviceManager dm;

  println("MotionPlusPlus running...");

  auto input_devices = dm.scan();

  if (!input_devices) {
    println("Error: {}", input_devices.error());
    return 1;
  }

  auto res = dm.populateMetadata(*input_devices);

  if (!res) {
    println("Error while populating metadata.");
    return 1;
  }

  for (auto &in_d : *input_devices) {
    println("{}", in_d);
  }

  try {
    DeviceConnection conn((*input_devices)[15]);

    while (true) {
      auto ev = conn.read();
      if (!ev) {
        if (ev.error() == "No event available.") continue;
        println("Error: {}", ev.error());
        return 1;
      }
      println("Type: {}, Code: {}, Value: {}", (*ev).type, (*ev).code, (*ev).value);
    }
  } catch (const std::exception& e){
    println("Error while connecting to device: {}", e.what());
    return 1;
  }

  return 0;
}
