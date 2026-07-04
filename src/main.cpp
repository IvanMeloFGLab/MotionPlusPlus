#include <print>
#include <cerrno>
#include "DeviceManager.hpp"
#include "DeviceConnection.hpp"
#include "WiiMote.hpp"

using std::println;
using std::vector;
using std::unique_ptr;
using std::string;
using std::make_unique;
using std::make_shared;
using std::move;

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

  auto groups = dm.groupByHid(*input_devices);

  /*vector<unique_ptr<Controller>> ctrls;
  int i = 1;

  for (auto &grp : groups) {
    if (grp.first.find("0005:057E:0306") != string::npos) { //Bluetooth:Nintendo:Wiimote
      ctrls.emplace_back(make_unique<WiiMote>(make_shared<DeviceManager>(dm), i, move(grp.second)));
      i++;
    }
  }*/

  auto ctrls = WiiMote::discover(make_shared<DeviceManager>(dm), 1, groups);

  println("Found {} Wiimotes.", ctrls.first.size());

  for (const auto &ctrl : ctrls.first) {
    println("{}", *ctrl);
  }

  if (ctrls.first.size() == 0) return 1;

  auto conn = ctrls.first[0]->connect();

  if (!conn) {
    println("Connection error: {}", conn.error().message());
    return 1;
  }

  while (true) {
    auto ev = ctrls.first[0]->read(2);
    if (!ev) {
      if (ev.error().value() == EAGAIN) continue;
      println("Event error: {}", ev.error().message());
      return 1;
    }
    println("Type: {}, Code: {}, Value: {}", ev->type, ev->code, ev->value);
  }

  /*while (true) {
    auto ev = conn->read();
    ;
  }*/

  return 0;
}
