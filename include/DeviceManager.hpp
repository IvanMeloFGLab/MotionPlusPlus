#pragma once

#include <filesystem>
#include <expected>
#include <vector>
#include <libevdev/libevdev.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <print>
#include <format>

struct InputDevice {
  std::filesystem::path path;
  std::string name;
  int vendor;
  int product;
  int bus;
  std::string phys;
  std::string uniq;

  explicit InputDevice(std::filesystem::path path) : path(path), name(""), vendor(0), product(0), bus(0), phys(""), uniq("") {}
};

template<>
struct std::formatter<InputDevice> : std::formatter<std::string> {
  auto format(const InputDevice &id, format_context &ctx) const {
    return std::formatter<std::string>::format(std::format("Name: {}, Vendor: {}, Product: {}, Bus: {}, Phys: {}, Uniq: {}",
                                                 id.name, id.vendor, id.product, id.bus, id.phys, id.uniq), ctx);
  }
};

class DeviceManager {
public:
  DeviceManager();
  ~DeviceManager();

  std::expected<std::vector<InputDevice>, std::error_code> scan();
  std::expected<void, std::error_code> populateMetadata(std::vector<InputDevice> &input_devices);
};
