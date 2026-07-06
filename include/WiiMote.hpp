#pragma once

#include "Controller.hpp"
#include <vector>
#include <memory>
#include <utility>
#include <unordered_map>

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
  int16_t x;
  int16_t y;
};

struct Ir {
  Point p1;
  Point p2;
  Point p3;
  Point p4;
  bool visible;
};

class WiiMote : public Controller {
private:
  WiiMote(std::shared_ptr<DeviceManager> dm, int ctrl_id, std::vector<std::unique_ptr<InputDevice>> devs);

public:
  static std::pair<std::unordered_map<int, std::unique_ptr<Controller>>, int> discover(std::shared_ptr<DeviceManager> dm, int ctrl_id_off, std::unordered_map<std::string, std::vector<std::unique_ptr<InputDevice>>> &grps);

  void update(int fd, input_event ev) override;

  WiiMote(WiiMote&& other);
  WiiMote(const WiiMote&) = delete;
  WiiMote& operator=(WiiMote&&) = delete;
  ~WiiMote();

  Buttons btns_;
  Accelerometer accel_;
  Gyroscope gyro_;
  Ir ir_;

private:
};
