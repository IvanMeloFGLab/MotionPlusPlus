#include <print>
#include <cerrno>
#include "DeviceManager.hpp"
#include "DeviceConnection.hpp"

using std::println;

int main() {

  DeviceManager dm;

  println("MotionPlusPlus running...");

  auto input_devices = dm.scan();

  if (!input_devices) {
    println("Scanning error: {}", input_devices.error().message());
    return 1;
  }

  auto res = dm.populateMetadata(*input_devices);

  if (!res) {
    println("Populating metadata error: {}. From {} device.", res.error().first.message(), res.error().second);
    return 1;
  }

  for (auto &in_d : *input_devices) {
    println("{}", in_d);
  }

  auto conn = DeviceConnection::connect((*input_devices)[15]);

  if (!conn) {
    println("Connection error: {}", conn.error().message());
    return 1;
  }

  auto groups = dm.groupByHid(*input_devices);

  for (const auto& [hid, vec] : groups) {
    for (const auto &in_d : vec) {
      println("Hid: {}, Dev: {}", hid, in_d->name);
    }
  }

  while (true) {
    auto ev = conn->read();
    if (!ev) {
      if (ev.error().value() == EAGAIN) continue;
      println("Event error: {}", ev.error().message());
      return 1;
    }
    println("Type: {}, Code: {}, Value: {}", ev->type, ev->code, ev->value);
  }

  return 0;
}
