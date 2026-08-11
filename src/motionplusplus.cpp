#include <ostream>
#include <print>
#include <iostream>
#include <chrono>
#include "libmotionplusplus/WiiMote.hpp"
#include "libmotionplusplus/ControllerManager.hpp"
#include <csignal>
#include <atomic>

#include "motionplusplus/VirtualController.hpp"

using std::println;
using std::signal;
using std::cout;

using namespace std::chrono_literals;
using namespace motionplusplus;

std::atomic<bool> running{true};

void quitHandle(int) {running = false; }

int main () {
    signal(SIGTERM, quitHandle);
    signal(SIGINT, quitHandle);

    ControllerManager cm;

    VirtualController vc(InputType::keyboard);

    auto op = vc.open();
    if (!op) {
        println("Virtual device could not be opened: {}", op.error().message());
        return 1;
    }

    bool a_btn_rel = false;

    while (running) {
        auto up = cm.update(10ms);

        if (cm.isNewContrllers()) {
            println("New device connected.");
            for (auto &ctrl_id : cm.getActiveControllers()) {
                println("{}", *(cm.getController(ctrl_id)));
            }
            cout.flush();
        }

        if (cm.getActiveControllers().size() > 0) {
            auto wm = dynamic_cast<WiiMote*>(cm.getController(1));
            if (wm == nullptr) {println("Could not load controller {} as a WiiMote.", 1); return 1;}
            bool a_btn = wm->getButtons().a;
            if (a_btn && !a_btn_rel) {
                auto vcup = vc.setKey(1);
                if (!vcup) {
                    println("Virtual controller error: {}", vcup.error().message());
                    running = false;
                    continue;
                }

                a_btn_rel = true;
            } else if (!a_btn && a_btn_rel) {
                auto vcup = vc.setKey(0);
                if (!vcup) {
                    println("Virtual controller error: {}", vcup.error().message());
                    running = false;
                    continue;
                }

                a_btn_rel = false;
            }
        }

        if (!up) {
            if (up.error().value() == EAGAIN || up.error().value() == EINTR) continue;
            if (up.error().value() == POLLHUP) {
                println("Device disconnected.");
                for (auto &ctrl_id : cm.getActiveControllers()) {
                    println("{}", *(cm.getController(ctrl_id)));
                }
                cout.flush();
                continue;
            }
            println("Update error: {}", up.error().message());
            cout.flush();

            return 1;
        }
    }

    return 0;
}
