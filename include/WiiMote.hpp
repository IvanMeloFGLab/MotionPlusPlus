#pragma once

#include "Controller.hpp"
#include <vector>
#include <memory>
#include <chrono>
#include <utility>
#include <map>
#include <thread>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <atomic>

struct Buttons {
  bool a;
  bool b;
  bool right;
  bool left;
  bool up;
  bool down;
  bool plus;
  bool home;
  bool minus;
  bool one;
  bool two;
};

struct Accelerometer {
  int16_t x;
  int16_t y;
  int16_t z;
};

struct Gyroscope {
  int16_t roll;
  int16_t pitch;
  int16_t yaw;
};

struct Point {
  int16_t x = 1023;
  int16_t y = 1023;
  bool visible() const { return x != 1023 && y != 1023; }
};

struct Ir {
  Point p1, p2, p3, p4;
};

struct Led {
  std::string index;
  std::string path;
  bool state;

  Led(std::string idx, std::string path) : index(idx), path(path), state(0) {}
  std::expected<void, std::error_code> set(bool on) {
    std::ofstream file(path + index + "/brightness");
    if (!file) return std::unexpected(std::error_code(errno, std::generic_category()));
    file << (on ? "1" : "0");
    state = on;
    return {};
  }
};

struct Leds {
  std::string path;
  Led l[4];

  Leds() : path(""), l{Led("0",""), Led("1",""), Led("2",""), Led("3","")} {}
  Leds(std::string path) : path(path), l{Led("0",path), Led("1",path), Led("2",path), Led("3",path)} {}

  Led& operator[](int i) {return l[i];}
};

class WiiMote : public Controller {
private:
  WiiMote(std::shared_ptr<DeviceManager> dm, int ctrl_id, std::vector<std::unique_ptr<InputDevice>> devs);

public:
  static std::pair<std::map<int, std::unique_ptr<Controller>>, int> discover(std::shared_ptr<DeviceManager> dm, std::vector<int> &bussyIds, std::map<std::string, std::vector<std::unique_ptr<InputDevice>>> &grps);

  WiiMote(WiiMote&& other);
  WiiMote(const WiiMote&) = delete;
  WiiMote& operator=(WiiMote&&) = delete;
  ~WiiMote();

  void update(int fd, input_event ev) override;
  std::expected<void, std::error_code> rumble(int intensity, std::chrono::milliseconds time, double freq=0, double offset=0);
  std::expected<void, std::error_code> animLed(std::chrono::milliseconds time, std::chrono::milliseconds delay=std::chrono::milliseconds(0));

  bool onLostFd(int fd) override;

  const Buttons getButtons() const;
  const Accelerometer getAccel() const;
  const Gyroscope getGyro() const;
  const Ir getIr() const;
  Leds getLeds();
  std::expected<int, std::error_code> getBatPer() const override;

  std::expected<void, std::error_code> setLedId(uint8_t id);

private:
  Buttons btns_;
  Accelerometer accel_;
  Gyroscope gyro_;
  Ir ir_;
  Leds leds_;

  std::thread leds_thread_;
  std::atomic<bool> stop_leds_{false};
  std::string bat_path_, leds_path_;
};
