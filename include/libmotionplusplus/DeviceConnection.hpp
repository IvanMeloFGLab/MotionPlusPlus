#pragma once

#include "DeviceManager.hpp"
#include <expected>
#include <libevdev/libevdev.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

namespace motionplusplus {
  class DeviceConnection {
  private:
    DeviceConnection(const InputDevice &device, int fd, libevdev *dev);

  public:
    DeviceConnection(DeviceConnection&& other);
    DeviceConnection(const DeviceConnection&) = delete;
    DeviceConnection& operator=(DeviceConnection&&) = delete;
    ~DeviceConnection() noexcept;

    static std::expected<DeviceConnection, std::error_code> connect(const InputDevice &device);
    std::expected<input_event, std::error_code> read();
    std::expected<int, std::error_code> uploadEffect(ff_effect &effect);
    std::expected<void, std::error_code> playEffect(int effect_id);
    std::expected<void, std::error_code> stopEffect(int effect_id);

    int getFd();
    const std::string getDeviceName();
    const InputDevice &getDevice() const;

  private:
    const InputDevice& device_;
    int fd_;
    libevdev *dev_;
    bool connected_;
  };
}
