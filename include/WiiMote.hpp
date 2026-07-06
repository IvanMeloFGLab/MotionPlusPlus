#pragma once

#include "Controller.hpp"
#include <vector>
#include <memory>
#include <utility>
#include <unordered_map>

class WiiMote : public Controller {
private:
  WiiMote(std::shared_ptr<DeviceManager> dm, int ctrl_id, std::vector<std::unique_ptr<InputDevice>> devs);

public:
  static std::pair<std::unordered_map<int, std::unique_ptr<Controller>>, int> discover(std::shared_ptr<DeviceManager> dm, int ctrl_id_off, std::unordered_map<std::string, std::vector<std::unique_ptr<InputDevice>>> &grps);

  WiiMote(WiiMote&& other);
  WiiMote(const WiiMote&) = delete;
  WiiMote& operator=(WiiMote&&) = delete;
  ~WiiMote();

private:
};
