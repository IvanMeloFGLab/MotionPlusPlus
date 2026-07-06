#pragma once

#include "Controller.hpp"
#include <vector>
#include <memory>
#include <expected>
#include <chrono>
#include <poll.h>
#include <cerrno>
#include <unordered_map>

class ControllerManager {
public:
  ControllerManager(std::unordered_map<int, std::unique_ptr<Controller>> ctrls);
  ~ControllerManager();

  std::expected<void, std::error_code> connect();
  std::expected<void, std::error_code> update(std::chrono::milliseconds timeout);

  std::unordered_map<int, std::unique_ptr<Controller>> ctrls_;

private:
  std::expected<void, std::error_code> updateFds();

  std::vector<pollfd> fds_;
};
