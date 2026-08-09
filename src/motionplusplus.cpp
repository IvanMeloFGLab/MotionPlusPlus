#include <print>
#include <chrono>
//#include "libmotionplusplus/WiiMote.hpp"
#include "libmotionplusplus/ControllerManager.hpp"

using std::println;

using namespace std::chrono_literals;
using namespace motionplusplus;

int main () {
    ControllerManager cm;

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
    }

    return 0;
}
