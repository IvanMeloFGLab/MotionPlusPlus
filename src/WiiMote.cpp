#include "WiiMote.hpp"

using std::vector;
using std::string;
using std::unique_ptr;
using std::shared_ptr;
using std::move;
using std::pair;
using std::make_pair;
using std::map;
using std::expected;
using std::unexpected;
using std::error_code;
using std::generic_category;
using std::thread;
using std::this_thread::sleep_for;
using std::chrono::steady_clock;
using std::chrono::duration;
using std::max;
using std::min;
using std::sin;
using std::cos;
using std::ifstream;
using std::ofstream;
using std::ranges::contains;

using milis = std::chrono::milliseconds;

WiiMote::WiiMote(shared_ptr<DeviceManager> dm, int ctrl_id, vector<std::unique_ptr<InputDevice>> devs) : Controller(dm, ctrl_id, move(devs)) {
  type_ = "Wiimote";
  bat_path_ = "/sys/class/power_supply/wiimote_battery_";
  leds_path_ = "/sys/class/leds/" + hid_ + ":blue:p";
  leds_ = Leds(leds_path_);
  stop_leds_ = false;

  animLed(milis(3000), milis(350));
}

WiiMote::WiiMote(WiiMote&& other) : Controller(move(other)) {

}

WiiMote::~WiiMote() {
  stop_leds_ = true;
  if (leds_thread_.joinable()) leds_thread_.join();
}

pair<map<int, unique_ptr<Controller>>, int> WiiMote::discover(shared_ptr<DeviceManager> dm, vector<int> &bussyIds,
                                                              map<string, vector<unique_ptr<InputDevice>>> &grps) {
  map<int, unique_ptr<Controller>> ctrls;
  int i = 1;

  for (auto &grp : grps) {
    if (grp.first.find("0005:057E:0306") != string::npos) { //Bluetooth:Nintendo:Wiimote
      while (contains(bussyIds, i)) i++;

      ctrls.emplace(i, unique_ptr<WiiMote>(new WiiMote(dm, i, move(grp.second))));
      bussyIds.push_back(i);
      i++;
    }
  }

  return make_pair(move(ctrls), i);
}

void WiiMote::update(int fd, input_event ev) {
  for (auto &conn : conns_) {
    if (conn.getFd() == fd) {
      if (ev.type == EV_SYN) {};
      if (ev.type == EV_KEY) {
        switch (ev.code) {
          case 304: {
            btns_.a = ev.value;
            break;
          } case 305: {
            btns_.b = ev.value;
            break;
          } case 106: {
            btns_.right = ev.value;
            break;
          } case 105: {
            btns_.left = ev.value;
            break;
          } case 103: {
            btns_.up = ev.value;
            break;
          } case 108: {
            btns_.down = ev.value;
            break;
          } case 407: {
            btns_.plus = ev.value;
            break;
          } case 316: {
            btns_.home = ev.value;
            break;
          } case 412: {
            btns_.minus = ev.value;
            break;
          } case 257: {
            btns_.one = ev.value;
            break;
          } case 258: {
            btns_.two = ev.value;
            break;
          }
        }
      }
      if (ev.type == EV_ABS) {
        if (conn.getDeviceName().find("Accelerometer") != string::npos) {
          switch (ev.code) {
            case 3: {
              accel_.x = ev.value;
              break;
            } case 4: {
              accel_.y = ev.value;
              break;
            } case 5: {
              accel_.z = ev.value;
              break;
            }
          }
        } else if (conn.getDeviceName().find("Motion Plus") != string::npos) {
          switch (ev.code) {
            case 3: {
              gyro_.roll = ev.value;
              break;
            } case 4: {
              gyro_.pitch = ev.value;
              break;
            } case 5: {
              gyro_.yaw = ev.value;
              break;
            }
          }
        } else if (conn.getDeviceName().find("IR") != string::npos) {
          switch (ev.code) {
            case 16: {
              ir_.p1.x = ev.value;
              break;
            } case 17: {
              ir_.p1.y = ev.value;
              break;
            } case 18: {
              ir_.p2.x = ev.value;
              break;
            } case 19: {
              ir_.p2.y = ev.value;
              break;
            } case 20: {
              ir_.p3.x = ev.value;
              break;
            } case 21: {
              ir_.p3.y = ev.value;
              break;
            } case 22: {
              ir_.p4.x = ev.value;
              break;
            } case 23: {
              ir_.p4.y = ev.value;
              break;
            }
          }
        }
      }
    }
  }
}

expected<void, error_code> WiiMote::rumble(int intensity, milis time, double freq, double offset) {
  ff_effect effect{};
  effect.type = FF_RUMBLE;
  effect.id = -1;                               // kernel assigns id
  effect.u.rumble.strong_magnitude = 0xFFFF;    // 0-100 → 0-0xFFFF
  effect.u.rumble.weak_magnitude   = 0xFFFF;
  effect.replay.length = time.count();
  effect.replay.delay = 0;

  for (auto &conn : conns_) {
    if (conn.getDeviceName() == "Nintendo Wii Remote") {
      auto id = conn.uploadEffect(effect);
      if (!id) return unexpected(id.error());

      auto play = conn.playEffect(*id);
      if (!play) return unexpected(play.error());

      if (freq == 0) {
        thread([conn = &conn, id = *id, time, intensity = min(intensity, 100)]() {
          auto end = steady_clock::now() + time;
          int on_time  = 25 + (intensity * 75 / 100);           // ms on
          int off_time = 100 - on_time;                         // ms off
          while (steady_clock::now() < end) {
            conn->playEffect(id);
            sleep_for(milis(on_time));
            conn->stopEffect(id);
            sleep_for(milis(off_time));
          }
          conn->stopEffect(id);
        }).detach();
      } else {
        thread([conn = &conn, id = *id, time, intensity = min(intensity, 100), freq, offset]() {
          auto end = steady_clock::now() + time;
          auto start = steady_clock::now();

          while (steady_clock::now() < end) {
            auto elapsed = duration<double>(steady_clock::now() - start).count();
            double sine = (sin((elapsed * freq * 2 * M_PI) + offset*M_PI) + 1.0) / 2.0;

            int on_time = 20 + (sine * intensity * 80 / 100);
            int off_time = 100 - on_time;

            conn->playEffect(id);
            sleep_for(milis(on_time));
            conn->stopEffect(id);
            sleep_for(milis(off_time));
          }
          conn->stopEffect(id);
        }).detach();
      }
    }
  }

  return {};
}

expected<void, error_code> WiiMote::animLed(milis time, milis delay) {
  leds_thread_ = thread([this, time, delay]() {
    sleep_for(milis(delay));
    auto end = time / 16;
    for (int i = 0; i <= 15 && !stop_leds_.load(); i++) {
      setLedId(i);
      sleep_for(milis(end));
    }
    if (!stop_leds_.load()) setLedId(ctrl_id_);
  });
  return {};
}

bool WiiMote::onLostFd(int fd) {
  for (auto it = conns_.begin(); it != conns_.end(); it++) {
    if (it->getFd() == fd) {
      if (it->getDeviceName().find("Motion Plus") != string::npos || it->getDeviceName().find("Nunchuk") != string::npos) {
        removeDeviceFor(*it);
        conns_.erase(it);
        return false;
      } else {
        return true;
      }
    }
  }
  return true;
}

const Buttons WiiMote::getButtons() const {
  return btns_;
}

const Accelerometer WiiMote::getAccel() const {
  return accel_;
}

const Gyroscope WiiMote::getGyro() const {
  return gyro_;
}

const Ir WiiMote::getIr() const {
  return ir_;
}

Leds WiiMote::getLeds() {
  return leds_;
}

expected<int, error_code> WiiMote::getBatPer() const {
  auto mac = getMac();
  if (!mac) return unexpected(mac.error());
  ifstream file(bat_path_ + *mac + "/capacity");
  if (!file) return unexpected(error_code(errno, generic_category()));
  string val;
  file >> val;
  return stoi(val);
}

expected<void, error_code> WiiMote::setLedId(uint8_t id) {
  for (uint8_t i = 0; i < 4; i++) {
    uint8_t mask = (1<<i);
    auto leds = leds_[i].set(id & mask);
    if (!leds) return unexpected(leds.error());
  }
  return {};
}
