#include <cstdint>
#include <ostream>
#include <print>
#include <iostream>
#include <chrono>
#include <csignal>
#include <atomic>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <vector>
#include <utility>
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
using std::clamp;
using std::pair;
using std::to_string;

using namespace std::chrono_literals;
using namespace motionplusplus;

struct Axis {
    uint16_t key;
    double trigger;
    int8_t direction;
};

struct ConfigMap {
    string name;

    unordered_map<int, pair<bool, unordered_map<string, uint16_t>>> btn2key_map;

    unordered_map<int, pair<bool, unordered_map<string, Axis>>> accel2key_map;
};

int full_find_or_default(const ConfigMap &map, const int &id) {
    int res  = map.btn2key_map.find(id) != map.btn2key_map.end() ||
                    map.accel2key_map.find(id) != map.accel2key_map.end() ? id :
                    map.btn2key_map.find(0) != map.btn2key_map.end() ||
                    map.accel2key_map.find(0) != map.accel2key_map.end() ? 0 : -1;

    return res;
}

template<typename K, typename V>
int find_or_default(const unordered_map<K, V> &map, const int &id) {
    int res  = map.find(id) != map.end() ? id : map.find(0) != map.end() ? 0 : -1;

    return res;
}

std::atomic<bool> running{true};

void quitHandle(int) {running = false; }

unordered_map<int, vector<VirtualController>> id2vc;

void println_id2vc() {
    println("Controller mapping table:");
    for (auto &id_vc : id2vc) {
        print(" - Ctrl {} -> [ ", id_vc.first);
        for (auto &vc : id_vc.second) print("{}", vc.getFd());
        println(" ]");
    }
}

ConfigMap map_from_toml(toml::table conf){
    ConfigMap map;

    auto name = conf["name"].value<string>();
    if (name.has_value()) {
        println("Loding: {}", *name);
        cout.flush();
        map.name = *name;
    } else {
        println("Config file has no config name.");
        cout.flush();
        map.name = "";
    }

    if (auto arr_wm = conf["wiimote"].as_array()) {
        for (auto&& node : *arr_wm) {
            auto wm_c = node.as_table();

            auto ctrl_id = (*wm_c)["ID"].value<int>().value_or(0);

            //Buttons
            if (auto conf_btns = (*wm_c)["buttons"].as_table()) {
                unordered_map<string, uint16_t> btns;
                for (auto btn : BTNS) {
                    if (auto key = (*conf_btns)[btn].value<string>()) {
                        if (key_code_map.find(key.value()) != key_code_map.end()) {
                            btns[btn] = key_code_map.at(key.value());
                        } else {
                            println("Warning: {} key is not supported.", key.value());
                            cout.flush();
                        }
                    }
                }

                map.btn2key_map.emplace(ctrl_id, pair((*conf_btns)["map_to"].value<string>().value() == "keyboard", std::move(btns)));
            } else {
                println("No buttons config detected for ID: {}.", ctrl_id == 0 ? "Default" : to_string(ctrl_id));
                cout.flush();
            }

            //Accel
            if (auto conf_accel = (*wm_c)["accel"].as_table()) {
                unordered_map<string, Axis> axs;
                for (auto axis : ACCEL) {
                    if (auto key = (*conf_accel)[axis]["key"].value<string>()) {
                        if (key_code_map.find(key.value()) != key_code_map.end()) {
                            axs[axis].key = key_code_map.at(key.value());
                            axs[axis].trigger = clamp((*conf_accel)[axis]["trigger"].value<double>().value_or(0.5), 0.0, 1.0);
                            axs[axis].direction = (*conf_accel)[axis]["trigger_direction"].value<string>().value_or("+/-") == "+/-" ? 0 :
                                                            (*conf_accel)[axis]["trigger_direction"].value<string>().value() == "+" ? 1 : -1;
                        } else {
                            println("Warning: {} key is not supported.", key.value());
                            cout.flush();
                        }
                    }
                }

                map.accel2key_map.emplace(ctrl_id, pair((*conf_accel)["map_to"].value<string>().value() == "keyboard", std::move(axs)));
            } else {
                println("No accelerometer config detected for ID: {}.", ctrl_id == 0 ? "Default" : to_string(ctrl_id));
                cout.flush();
            }
        }
    } else {
        println("Warning: No mapping detected for WiiMotes.");
        cout.flush();
    }

    return map;
}

int main () {
    signal(SIGTERM, quitHandle);
    signal(SIGINT, quitHandle);

    ControllerManager cm;

    auto fpth = std::filesystem::path(std::getenv("HOME")) / ".config" / "motionplusplus";
    auto config = std::filesystem::exists(fpth / "conf.toml") ? toml::parse_file((fpth / "conf.toml").string()) : toml::parse_file((fpth / "conf.example.toml").string());
    auto mapping = map_from_toml(config);

    while (running) {
        auto up = cm.update(10ms);

        if (cm.isNewContrllers()) {
            println("New device connected.");
            for (auto &ctrl_id : cm.getActiveControllers()) {
                println("{}", *(cm.getController(ctrl_id)));

                //Virtual devices creation.
                auto [it, inserted] = id2vc.try_emplace(ctrl_id);
                if (inserted) {
                    auto set = false;

                    auto tmp_id = full_find_or_default(mapping, ctrl_id);
                    if (tmp_id == -1) continue;

                    if (mapping.btn2key_map.find(tmp_id) != mapping.btn2key_map.end()) {
                        if  (mapping.btn2key_map.at(tmp_id).first) {
                            it->second.emplace_back(InputType::keyboard);
                            set = true;
                        }
                    }

                    if  ((mapping.accel2key_map.find(tmp_id) != mapping.accel2key_map.end()) && !set) {
                        if (mapping.accel2key_map.at(tmp_id).first) {
                            it->second.emplace_back(InputType::keyboard);
                            set = true;
                        }
                    }

                    if (!set) continue;

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
            if (cm.getController(id)->getType() == "wiimote" ) {
                auto wm = dynamic_cast<WiiMote*>(cm.getController(id));
                if (wm == nullptr) {println("Could not load controller {} as a WiiMote.", id); cout.flush(); return 1;}

                std::unordered_map<uint16_t, bool> desired;

                auto tmp_id = full_find_or_default(mapping, id);
                if (tmp_id == -1) continue;

                if (mapping.btn2key_map.find(tmp_id) != mapping.btn2key_map.end()) {
                    if (mapping.btn2key_map.at(tmp_id).first) {
                        const auto& btn_map = mapping.btn2key_map.at(tmp_id).second;

                        auto btns = wm->getButtons();
                        for (const auto &m : btns) {
                            desired[btn_map.at(m.first)] |= m.second;
                        }
                    }
                }

                if (mapping.accel2key_map.find(tmp_id) != mapping.accel2key_map.end()) {
                    if (mapping.accel2key_map.at(tmp_id).first) {
                        const auto& accel_map = mapping.accel2key_map.at(tmp_id).second;

                        auto accel = wm->getAccel();
                        //println("x: {}, y: {}, z: {}", accel.at("x"), accel.at("y"), accel.at("z"));
                        for (const auto &a : accel) {
                            if (accel_map.find(a.first) == accel_map.end()) continue;

                            auto ntrigg = accel_map.at(a.first).trigger;
                            auto dir = accel_map.at(a.first).direction;
                            bool trigg = false;

                            switch (dir) {
                                case 0: {
                                    trigg = a.second >= 500 * ntrigg || a.second <= -500 * ntrigg;
                                    break;
                                } case 1: {
                                    trigg = a.second >= 500 * ntrigg;
                                    break;
                                } case -1: {
                                    trigg = a.second <= -500 * ntrigg;
                                    break;
                                }
                            }

                            desired[accel_map.at(a.first).key] |= trigg;
                        }
                    }
                }

                for (const auto &[key_num, state] : desired) {
                    auto vcup = id2vc.at(id).back().setKey(key_num, state);
                    if (!vcup) {
                        println("Virtual controller error: {}", vcup.error().message());
                        cout.flush();
                        running = false;
                        continue;
                    }
                }

                auto vcsy = id2vc.at(id).back().sync();
                if (!vcsy) {
                    println("Virtual controller sync error: {}", vcsy.error().message());
                    cout.flush();
                    running = false;
                    continue;
                }
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
