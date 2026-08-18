#include <cstdint>
#include <ostream>
#include <print>
#include <iostream>
#include <chrono>
#include <csignal>
#include <atomic>
#include <unordered_map>
#include <filesystem>
#include <vector>
#include <algorithm>

#include "libmotionplusplus/WiiMote.hpp"
#include "libmotionplusplus/ControllerManager.hpp"

#include <toml++/toml.hpp>

#include "motionplusplus/VirtualController.hpp"
#include "motionplusplus/KeyCodeMap.hpp"

using std::println;
using std::print;
using std::signal;
using std::cout;
using std::string;
using std::vector;
using std::unordered_map;

using namespace std::chrono_literals;
using namespace motionplusplus;

std::atomic<bool> running{true};

void quitHandle(int) {running = false; }

unordered_map<int, vector<VirtualController>> id2vc;
unordered_map<uint16_t, int> key_cnt;

void println_id2vc() {
    println("Controller mapping table:");
    for (auto &id_vc : id2vc) {
        print(" - Ctrl {} -> [ ", id_vc.first);
        for (auto &vc : id_vc.second) print("{}", vc.getFd());
        println(" ]");
    }
}

int main () {
    signal(SIGTERM, quitHandle);
    signal(SIGINT, quitHandle);

    ControllerManager cm;

    auto fpth = std::filesystem::path(std::getenv("HOME")) / ".config" / "motionplusplus";
    auto config = std::filesystem::exists(fpth / "conf.toml") ? toml::parse_file((fpth / "conf.toml").string()) : toml::parse_file((fpth / "conf.example.toml").string());
    auto name = config["name"].value<std::string>();
    if (name.has_value()) {
        println("Loding: {}", *name);
        cout.flush();
    } else {
        println("Config file has no config name.");
        cout.flush();
    }

    while (running) {
        auto up = cm.update(10ms);

        if (cm.isNewContrllers()) {
            println("New device connected.");
            for (auto &ctrl_id : cm.getActiveControllers()) {
                println("{}", *(cm.getController(ctrl_id)));

                auto [it, inserted] = id2vc.try_emplace(ctrl_id);
                if (inserted) {
                    it->second.emplace_back(InputType::keyboard);

                    auto op = it->second.back().open();
                    if (!op) {
                        println("Virtual device could not be opened: {}", op.error().message());
                        cout.flush();
                        return 1;
                    }
                }
            }
            println_id2vc();
            cout.flush();
        }

        for (auto &id : cm.getActiveControllers()) {
            auto wm = dynamic_cast<WiiMote*>(cm.getController(id));
            if (wm == nullptr) {println("Could not load controller {} as a WiiMote.", id); cout.flush(); return 1;}
            auto btns = wm->getButtons();
            std::unordered_map<uint16_t, bool> desired;
            for (const auto &m : btns) {
                auto key = config["wiimote"][m.first].value<std::string>();
                uint16_t key_num = key_code_map.find(key.value_or("KEY_A")) != key_code_map.end() ? key_code_map.at(key.value_or("KEY_A")) : KEY_A;
                desired[key_num] |= *m.second;
            }
            for (const auto &[key_num, state] : desired) {
                auto vcup = id2vc[id].back().setKey(key_num, state);
                if (!vcup) {
                    println("Virtual controller error: {}", vcup.error().message());
                    cout.flush();
                    running = false;
                    continue;
                }
            }

            //auto accel = wm->getAccel();
            //println("x: {}, y: {}, z: {}", accel.x, accel.y, accel.z);

            auto vcsy = id2vc[id].back().sync();
            if (!vcsy) {
                println("Virtual controller sync error: {}", vcsy.error().message());
                cout.flush();
                running = false;
                continue;
            }
        }

        if (!up) {
            if (up.error().value() == EAGAIN || up.error().value() == EINTR) continue;
            if (up.error().value() == EBUSY || up.error().value() == ENODEV || up.error().value() == ENOTCONN) {
                println("Device disconnected.");
                for (auto &ctrl_id : cm.getActiveControllers()) {
                    println("{}", *(cm.getController(ctrl_id)));
                }
                cout.flush();
                vector<int> id2er;
                for (auto &id : id2vc) {
                    auto actrls = cm.getActiveControllers();
                    if (std::find(actrls.begin(), actrls.end(), id.first) == actrls.end()) {
                        id2er.push_back(id.first);
                    }
                }
                for (auto &id : id2er) id2vc.erase(id);
                println_id2vc();
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
