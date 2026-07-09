#pragma once

#include "DeviceManager.hpp"
#include "Controller.hpp"
#include "WiiMote.hpp"
#include <vector>
#include <memory>
#include <expected>
#include <chrono>
#include <poll.h>
#include <cerrno>
#include <map>
#include <utility>
#include <unordered_set>

namespace motionplusplus {
  class ControllerManager {
  public:
    ControllerManager();
    ~ControllerManager();

    std::expected<std::vector<std::pair<int, std::string>>, std::error_code> scan();
    std::expected<void, std::error_code> connect();
    std::expected<void, std::error_code> update(std::chrono::milliseconds timeout);
    std::expected<void, std::error_code> disconnect(std::pair<const int, std::unique_ptr<Controller>> &ctrl, int fd);

    std::vector<int> getActiveControllers();
    Controller* getController(int id);

    bool isNewContrllers();

  private:
    std::expected<void, std::error_code> updateFds();
    std::expected<std::vector<InputDevice>, std::error_code> lightScan();

    DeviceManager dm_;
    std::map<int, std::unique_ptr<Controller>> ctrls_;
    std::vector<pollfd> fds_;
    int device_num_;
    std::chrono::milliseconds scan_interval_;
    std::chrono::steady_clock::time_point last_scan_;
    bool new_controllers_;
  };
}
