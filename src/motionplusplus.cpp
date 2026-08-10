#include <ostream>
#include <print>
#include <iostream>
#include <chrono>
//#include "libmotionplusplus/WiiMote.hpp"
#include "libmotionplusplus/ControllerManager.hpp"
#include <csignal>
#include <atomic>

using std::println;
using std::signal;
using std::cout;

using namespace std::chrono_literals;
using namespace motionplusplus;

std::atomic<bool> running{true};

void quitHandle(int) { running = false; }

int main () {
    signal(SIGTERM, quitHandle);
    signal(SIGINT, quitHandle);

    ControllerManager cm;

    while (running) {
        auto up = cm.update(10ms);

        if (cm.isNewContrllers()) {
            println("New device connected.");
            for (auto &ctrl_id : cm.getActiveControllers()) {
                println("{}", *(cm.getController(ctrl_id)));
            }
            cout.flush();
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
