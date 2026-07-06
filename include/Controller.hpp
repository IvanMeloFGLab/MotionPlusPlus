#pragma once

#include "DeviceManager.hpp"
#include "DeviceConnection.hpp"
#include <vector>
#include <memory>
#include <format>
#include <expected>
#include <libevdev/libevdev.h>
#include <utility>
#include <unordered_map>

class Controller {
protected:
  Controller(std::shared_ptr<DeviceManager> dm, int ctrl_id, std::vector<std::unique_ptr<InputDevice>> devs);

public:
  Controller(Controller&& other) = default;
  Controller(const Controller&) = delete;
  Controller& operator=(Controller&&) = delete;
  virtual ~Controller() noexcept;

  //virtual std::pair<std::vector<std::unique_ptr<Controller>>, int> discover(std::shared_ptr<DeviceManager> &dm, int &ctrl_id_off, std::unordered_map<std::string, std::vector<std::unique_ptr<InputDevice>>> &grps) = 0;
  std::expected<void, std::error_code> connect();
  std::expected<input_event, std::error_code> read(int dev_num);
  virtual void update(int fd, input_event ev) = 0;

  std::vector<std::string> getDevicesNames() const;
  int getId() const;
  std::string getHid() const;

  std::vector<int> getFds();

  bool isConnected();
  bool isFdFromCtrl(int &fd);

private:

protected:
  std::shared_ptr<DeviceManager> dm_;
  int ctrl_id_;
  std::vector<std::unique_ptr<InputDevice>> devs_;
  std::string hid_;
  bool connected_;
  std::vector<DeviceConnection> conns_;
};

template<>
struct std::formatter<Controller> : std::formatter<std::string> {
  auto format(const Controller &ctrl, format_context &ctx) const {
    std::string tmp;
    for (const auto &name : ctrl.getDevicesNames()) tmp += std::format("    - {}\n", name);
    return std::formatter<std::string>::format(std::format("Controller {}, {}\n", ctrl.getId(), ctrl.getHid()) + tmp, ctx);
  }
};
