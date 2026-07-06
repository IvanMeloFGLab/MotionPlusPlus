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
#include <unordered_map>
#include <utility>

class ControllerManager {
public:
  ControllerManager();
  ~ControllerManager();

  std::expected<std::vector<std::pair<int, std::string>>, std::error_code> scan();
  std::expected<void, std::error_code> connect();
  std::expected<void, std::error_code> update(std::chrono::milliseconds timeout);

  Controller* getController(int id);

private:
  std::expected<void, std::error_code> updateFds();

  DeviceManager dm_;
  std::unordered_map<int, std::unique_ptr<Controller>> ctrls_;
  std::vector<pollfd> fds_;
};
