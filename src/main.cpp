#include <print>
#include <chrono>
#include "WiiMote.hpp"
#include "ControllerManager.hpp"

using std::println;

using namespace std::chrono_literals;

int main() {
  ControllerManager cm;

  auto scn = cm.scan();
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
  }

  while (true) {
    auto up = cm.update(10ms);

    if (!up) {
      if (up.error().value() == EAGAIN) continue;
      println("Update error: {}", up.error().message());
      return 1;
    }

    auto wm = dynamic_cast<WiiMote*>(cm.getController(1));
    auto wm2 = dynamic_cast<WiiMote*>(cm.getController(2));
    if (wm && wm2) {
      println("A button {} 1: {}, {} 2: {}", wm->getType(), wm->getButtons().a, wm2->getType(), wm2->getButtons().a);
    }

  }

  return 0;
}
