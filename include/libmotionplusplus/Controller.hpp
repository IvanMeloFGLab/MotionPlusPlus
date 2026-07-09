#pragma once

#include "DeviceManager.hpp"
#include "DeviceConnection.hpp"
#include <vector>
#include <list>
#include <memory>
#include <format>
#include <expected>
#include <libevdev/libevdev.h>
#include <utility>
#include <unordered_map>
#include <fstream>

namespace motionplusplus {
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

    virtual bool onLostFd(int fd) = 0;
    void removeDeviceFor(const DeviceConnection& conn);

    void addDevices(std::vector<std::unique_ptr<InputDevice>> devs);

    std::vector<std::string> getDevicesNames() const;
    int getId() const;
    std::string getHid() const;
    std::expected<std::string, std::error_code> getMac() const;
    std::string getType() const;
    virtual std::expected<int, std::error_code> getBatPer() const = 0;
    std::vector<int> getFds();

    void setConnected(bool conn);

    bool isConnected();
    bool isFdFromCtrl(int &fd);

  private:

  protected:
    std::shared_ptr<DeviceManager> dm_;
    int ctrl_id_;
    std::vector<std::unique_ptr<InputDevice>> devs_;
    std::string hid_, type_;
    bool connected_;
    std::list<DeviceConnection> conns_;
  };
}

template<>
struct std::formatter<motionplusplus::Controller> : std::formatter<std::string> {
  auto format(const motionplusplus::Controller &ctrl, format_context &ctx) const {
    auto bat = ctrl.getBatPer();
    std::string tmp = bat ? std::format("Battery: {}%\n", *bat) : "Battery: unknown\n";
    for (const auto &name : ctrl.getDevicesNames()) tmp += std::format("    - {}\n", name);
    return std::formatter<std::string>::format(std::format("{} controller {}, {}\n", ctrl.getType(), ctrl.getId(), ctrl.getHid()) + tmp, ctx);
  }
};
