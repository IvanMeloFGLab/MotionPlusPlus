#pragma once

#include "Controller.hpp"
#include <vector>
#include <memory>

class WiiMote : public Controller {
public:
  WiiMote(std::shared_ptr<DeviceManager> dm, int ctrl_id, std::vector<std::unique_ptr<InputDevice>> devs);
  ~WiiMote();

private:
};
