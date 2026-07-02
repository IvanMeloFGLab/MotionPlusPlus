#pragma once

#include "DeviceManager.hpp"
#include <expected>
#include <libevdev/libevdev.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

using std::runtime_error;
using std::expected;
using std::unexpected;

class DeviceConnection {
public:
  DeviceConnection(const InputDevice &device);
  ~DeviceConnection();

  expected<input_event, string> read();

private:
  const InputDevice& device_;
  libevdev *dev_;
  int fd_, rc_;
};
