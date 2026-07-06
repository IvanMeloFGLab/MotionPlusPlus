#pragma once

#include "DeviceManager.hpp"
#include <expected>
#include <libevdev/libevdev.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

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

  int getFd();
  const std::string getDeviceName();

private:
  const InputDevice& device_;
  int fd_;
  libevdev *dev_;
};
