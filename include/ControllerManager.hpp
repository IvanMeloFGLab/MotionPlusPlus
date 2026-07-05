#pragma once

#include "Controller.hpp"
#include <vector>
#include <memory>
#include <expected>

class ControllerManager {
public:
  ControllerManager(std::vector<std::unique_ptr<Controller>> ctrls);
  ~ControllerManager();

  std::expected<void, std::error_code> connect();

private:
  std::vector<std::unique_ptr<Controller>> ctrls_;
  bool connected_;
};
