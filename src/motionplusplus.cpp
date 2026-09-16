#include <cerrno>
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
using std::find_if;

using namespace std::chrono_literals;
using namespace motionplusplus;

enum class InputConf {
    keyboard,
    mouse,
    keymouse,
    none
};

struct Axis {
    uint16_t key;
    double trigger;
    int8_t direction;
};

struct IrConf {
    bool mode;
    double sensitivity;
};

struct ConfigMap {
    string name;

    unordered_map<int, pair<InputConf, unordered_map<string, uint16_t>>> btn2key;
    unordered_map<int, pair<bool, unordered_map<string, Axis>>> accel2key;
    unordered_map<int, pair<bool, IrConf>> ir2mouse;
};

int full_find_or_default(const ConfigMap &map, const int &id) {
    int res  = map.btn2key.find(id) != map.btn2key.end() ||
                    map.accel2key.find(id) != map.accel2key.end() ||
                    map.ir2mouse.find(id) != map.ir2mouse.end() ? id :
                    map.btn2key.find(0) != map.btn2key.end() ||
                    map.accel2key.find(0) != map.accel2key.end() ||
                    map.ir2mouse.find(0) != map.ir2mouse.end() ? 0 : -1;

    return res;
}

std::atomic<bool> running{true};

void quitHandle(int) {running = false; }

unordered_map<int, vector<VirtualController>> id2vc;
unordered_map<int, Point> id2last_point;

void println_id2vc() {
    println("Controller mapping table:");
    for (auto &id_vc : id2vc) {
        print(" - Ctrl {} -> [ ", id_vc.first);
        for (auto &vc : id_vc.second) print("{}/", vc.getFd());
        println(" ]");
    }
}

InputConf str2tp(string tp) {
    if (tp == "keyboard") return InputConf::keyboard;
    if (tp == "mouse") return InputConf::mouse;
    if (tp == "mouse+keyboard") return InputConf::keymouse;
    return InputConf::none;
}

InputType InputConf2InputType (InputConf ip, bool mode) {
    if (ip == InputConf::keyboard) return InputType::keyboard;
    if (ip == InputConf::mouse && mode) return InputType::rel_mouse;
    if (ip == InputConf::mouse && !mode) return InputType::abs_mouse;
    return InputType::none;
}

VirtualController* findVc(vector<VirtualController> &vcs, InputConf type) {
    auto it = find_if(vcs.begin(), vcs.end(), [type](const VirtualController &vc) { return vc.getType() == InputConf2InputType(type, true) ||
                                                                                                                                         vc.getType() == InputConf2InputType(type, false); });
    return it != vcs.end() ? &(*it) : nullptr;
}

VirtualController* findVcSp(vector<VirtualController> &vcs, InputType type) {
    auto it = find_if(vcs.begin(), vcs.end(), [type](const VirtualController &vc) { return vc.getType() == type; });
    return it != vcs.end() ? &(*it) : nullptr;
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

                if (auto map_to = (*conf_btns)["map_to"].value<string>()) {
                    auto tp =str2tp(map_to.value());
                    if (tp != InputConf::none) map.btn2key.emplace(ctrl_id, pair(tp, std::move(btns))); else {
                        println("Warning no valid map_to config detected for ID: {}.", ctrl_id == 0 ? "Default" : to_string(ctrl_id));
                        cout.flush();
                    }
                } else {
                    println("Warning no buttons map_to config detected for ID: {}.", ctrl_id == 0 ? "Default" : to_string(ctrl_id));
                    cout.flush();
                }

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

                map.accel2key.emplace(ctrl_id, pair((*conf_accel)["map_to"].value<string>().value_or("") == "keyboard", std::move(axs)));
            } else {
                println("No accelerometer config detected for ID: {}.", ctrl_id == 0 ? "Default" : to_string(ctrl_id));
                cout.flush();
            }

            //IR
            if (auto conf_ir = (*wm_c)["ir"].as_table()) {
                IrConf mv;
                mv.mode = (*conf_ir)["mode"].value<string>().value_or("relative") == "relative";
                mv.sensitivity = (*conf_ir)["sensitivity"].value<double>().value_or(1.0);

                map.ir2mouse.emplace(ctrl_id, pair((*conf_ir)["map_to"].value<string>().value_or("") == "mouse", std::move(mv)));
            } else {
                println("No IR config detected for ID: {}.", ctrl_id == 0 ? "Default" : to_string(ctrl_id));
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

        // Find new controllers
        if (cm.isNewContrllers()) {
            println("New device connected.");
            for (auto &ctrl_id : cm.getActiveControllers()) {
                println("{}", *(cm.getController(ctrl_id)));

                //Virtual devices creation.
                auto [it, inserted] = id2vc.try_emplace(ctrl_id);
                if (inserted) {
                    auto tmp_id = full_find_or_default(mapping, ctrl_id);
                    if (tmp_id == -1) continue;

                    if (mapping.btn2key.find(tmp_id) != mapping.btn2key.end()) {
                        if (mapping.btn2key.at(tmp_id).first == InputConf::keymouse){
                            if (findVc(it->second, InputConf::keyboard) == nullptr) it->second.emplace_back(InputType::keyboard);
                            if (findVc(it->second, InputConf::mouse) == nullptr) it->second.emplace_back(InputType::rel_mouse);
                        } else {
                            if (findVc(it->second, mapping.btn2key.at(tmp_id).first) == nullptr) it->second.emplace_back(InputConf2InputType(mapping.btn2key.at(tmp_id).first, true));
                        }
                    }

                    if  (mapping.accel2key.find(tmp_id) != mapping.accel2key.end()) {
                        if (mapping.accel2key.at(tmp_id).first) {
                            if (findVc(it->second, InputConf::keyboard) == nullptr) it->second.emplace_back(InputType::keyboard);
                        }
                    }

                    if ((mapping.ir2mouse.find(tmp_id) != mapping.ir2mouse.end())) {
                        if(mapping.ir2mouse.at(tmp_id).first) {
                            if (findVc(it->second, InputConf::mouse) == nullptr) {
                                if (mapping.ir2mouse.at(tmp_id).second.mode) it->second.emplace_back(InputType::rel_mouse);
                                else it->second.emplace_back(InputType::abs_mouse);
                            } else {
                                if (auto fvc = findVcSp(it->second, !mapping.ir2mouse.at(tmp_id).second.mode ? InputType::rel_mouse : InputType::abs_mouse)) {
                                    VirtualController nvc(mapping.ir2mouse.at(tmp_id).second.mode ? InputType::rel_mouse : InputType::abs_mouse);
                                    *fvc = std::move(nvc);
                                }
                            }
                        }
                    }

                    for (auto &vc : it->second) {
                        auto op = vc.open();
                        if (!op) {
                            println("Virtual device could not be opened: {}", op.error().message());
                            cout.flush();
                            return 1;
                        }
                    }
                }
            }
            println_id2vc();
            cout.flush();
        }

        // Update state
        for (auto &id : cm.getActiveControllers()) {
            if (cm.getController(id)->getType() == "wiimote" ) {
                auto wm = dynamic_cast<WiiMote*>(cm.getController(id));
                if (wm == nullptr) {println("Could not load controller {} as a WiiMote.", id); cout.flush(); return 1;}

                auto tmp_id = full_find_or_default(mapping, id);
                if (tmp_id == -1) continue;

                auto key_set = false;
                auto mos_set = false;
                auto mos_key = false;

                std::unordered_map<uint16_t, bool> desired;


                if (mapping.btn2key.find(tmp_id) != mapping.btn2key.end()) {
                    const auto& btn_map = mapping.btn2key.at(tmp_id).second;

                    auto btns = wm->getButtons();
                    for (const auto &m : btns) {
                        if (btn_map.find(m.first) != btn_map.end()) {
                            desired[btn_map.at(m.first)] |= m.second;
                            if (mapping.btn2key.at(tmp_id).first == InputConf::keymouse) {
                                mos_key = mos_set = key_set = true;
                            } else {
                                auto in = mapping.btn2key.at(tmp_id).first;
                                if (in == InputConf::keyboard) key_set = true;
                                if (in == InputConf::mouse) mos_key = mos_set = true;
                            }
                        }
                    }
                }

                if (mapping.accel2key.find(tmp_id) != mapping.accel2key.end()) {
                    if (mapping.accel2key.at(tmp_id).first) {
                        const auto& accel_map = mapping.accel2key.at(tmp_id).second;

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
                            key_set = true;
                        }
                    }
                }

                if (key_set) {
                    if (auto *kb = findVc(id2vc.at(id), InputConf::keyboard)) {
                        for (const auto &[key_num, state] : desired) {
                            auto vcup = kb->setKey(key_num, state);
                            if (!vcup) {
                                if (vcup.error().value() == ENOENT) {
                                    continue;
                                } else {
                                    println("Virtual controller error: {}", vcup.error().message());
                                    cout.flush();
                                    running = false;
                                    continue;
                                }
                            }
                        }

                        auto vcsy = kb->sync();
                        if (!vcsy) {
                            println("Virtual controller sync error: {}", vcsy.error().message());
                            cout.flush();
                            running = false;
                            continue;
                        }
                    }
                }

                pair <int32_t, int32_t> moveTo = {0, 0};

                if (mapping.ir2mouse.find(tmp_id) != mapping.ir2mouse.end()) {
                    if (mapping.ir2mouse.at(tmp_id).first) {
                        auto& ir_conf = mapping.ir2mouse.at(tmp_id).second;

                        auto ir  = wm->getIr();
                        Point new_point;
                        //println("IR = p1: {}-{}, p2: {}-{}, p3: {}-{}, p4: {}-{}", ir.p1.x, ir.p1.y, ir.p2.x, ir.p2.y, ir.p3.x, ir.p3.y, ir.p4.x, ir.p4.y);
                        if (ir.p1.visible() && ir.p2.visible()) {
                            if (ir_conf.mode) {
                                new_point.x = ir.p1.x - id2last_point[id].x;
                                new_point.y = ir.p1.y - id2last_point[id].y;

                                id2last_point[id].x = ir.p1.x;
                                id2last_point[id].y = ir.p1.y;

                                moveTo.first = -new_point.x * ir_conf.sensitivity;
                                moveTo.second = new_point.y * ir_conf.sensitivity;
                                mos_set = true;
                            } else {
                                moveTo.first = 1023 - ir.p1.x;
                                moveTo.second = ir.p1.y;
                            }
                        }
                    }
                }

                if (mos_set) {
                    if (auto *ms = findVc(id2vc.at(id), InputConf::mouse)) {
                        if (mos_key) {
                            for (const auto &[key_num, state] : desired) {
                                auto vcup = ms->setKey(key_num, state);
                                if (!vcup) {
                                    if (vcup.error().value() == ENOENT) {
                                        continue;
                                    } else {
                                        println("Virtual controller error: {}", vcup.error().message());
                                        cout.flush();
                                        running = false;
                                        continue;
                                    }
                                }
                            }
                        }

                        if (ms->getType() == InputType::rel_mouse) {
                            auto vcup = ms->moveRel(REL_X, moveTo.first);
                            auto vcupy = ms->moveRel(REL_Y, moveTo.second);
                            if (!vcup) {
                                if (vcup.error().value() == ENOENT) {
                                    continue;
                                } else {
                                    println("Virtual controller error: {}", vcup.error().message());
                                }
                                cout.flush();
                                running = false;
                                continue;
                            }

                            if (!vcupy) {
                                if (vcupy.error().value() == ENOENT) {
                                    continue;
                                } else {
                                    println("Virtual controller error: {}", vcupy.error().message());
                                }
                                cout.flush();
                                running = false;
                                continue;
                            }
                        }

                        if (ms->getType() == InputType::abs_mouse) {
                            auto vcup = ms->moveAbs(ABS_X, moveTo.first);
                            auto vcupy = ms->moveAbs(ABS_Y, moveTo.second);
                            if (!vcup) {
                                if (vcup.error().value() == ENOENT) {
                                    continue;
                                } else {
                                    println("Virtual controller error: {}", vcup.error().message());
                                }
                                cout.flush();
                                running = false;
                                continue;
                            }

                            if (!vcupy) {
                                if (vcupy.error().value() == ENOENT) {
                                    continue;
                                } else {
                                    println("Virtual controller error: {}", vcupy.error().message());
                                }
                                cout.flush();
                                running = false;
                                continue;
                            }
                        }

                        auto vcsy = ms->sync();
                        if (!vcsy) {
                            println("Virtual controller sync error: {}", vcsy.error().message());
                            cout.flush();
                            running = false;
                            continue;
                        }
                    }
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
