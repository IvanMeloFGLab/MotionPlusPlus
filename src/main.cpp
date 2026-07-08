#include <print>
#include <chrono>
#include "WiiMote.hpp"
#include "ControllerManager.hpp"

using std::println;

using namespace std::chrono_literals;

int main() {
  ControllerManager cm;

  /*auto scn = cm.scan();
  if (!scn) {
    println("Scanning error: {}", scn.error().message());
    return 1;
  }

  for (const auto &ctrl : *scn) {
    println("{}", *(cm.getController(ctrl.first)));
  }

  auto conn = cm.connect();

  if (!conn) {
    println("Connection error: {}", conn.error().message());
    return 1;
  }*/

  //auto wm = dynamic_cast<WiiMote*>(cm.getController(1));
  //auto wm2 = dynamic_cast<WiiMote*>(cm.getController(2));

  //wm->rumble(100, 5000ms, 1.0);
  //wm2->rumble(100, 5000ms, 1.0, 1.0);
  //auto led = wm->animLed(2500ms);
  //if (!led) {println("Leds error: {}", led.error().message());}
  //wm2->rumbleCosine(40, 3000ms, 1.5);

  while (true) {
    auto up = cm.update(10ms);

    if (cm.isNewContrllers()) {
      println("New device connected.");
      for (auto &ctrl_id : cm.getActiveControllers()) {
        println("{}", *(cm.getController(ctrl_id)));
      }
    }

    if (!up) {
      if (up.error().value() == EAGAIN) continue;
      if (up.error().value() == POLLHUP) {
        println("Device disconnected.");
        for (auto &ctrl_id : cm.getActiveControllers()) {
          println("{}", *(cm.getController(ctrl_id)));
        }
        continue;
      }
      println("Update error: {}", up.error().message());
      return 1;
    }

    //auto wm = dynamic_cast<WiiMote*>(cm.getController(1));
    //auto wm2 = dynamic_cast<WiiMote*>(cm.getController(2));

    /*if (wm && wm2) {
      println("A button {} 1: {}, {} 2: {}", wm->getType(), wm->getButtons().a, wm2->getType(), wm2->getButtons().a);
    }*/

  }

  return 0;
}
