#pragma once

#include <filesystem>
#include <expected>
#include <vector>
#include <libevdev/libevdev.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <format>
#include <utility>
#include <map>
#include <memory>

struct InputDevice {
  std::filesystem::path path;
  std::string name;
  int vendor;
  int product;
  int bus;
  std::string phys;
  std::string uniq;
  std::string hid;

  explicit InputDevice(std::filesystem::path path) : path(path), name(""), vendor(0), product(0), bus(0), phys(""), uniq(""), hid("") {}
};

template<>
struct std::formatter<InputDevice> : std::formatter<std::string> {
  auto format(const InputDevice &id, format_context &ctx) const {
    return std::formatter<std::string>::format(std::format("Name: {}, Vendor: {}, Product: {}, Bus: {}, Phys: {}, Uniq: {}, hid: {}",
                                                          id.name, id.vendor, id.product, id.bus, id.phys, id.uniq, id.hid), ctx);
  }
};

enum class DeviceManagerError {
  NoHIDFound
};

struct DeviceManagerErrorCategory : std::error_category {
  const char* name() const noexcept override { return "device_manager"; }

  std::string message(int e) const override {
    switch (static_cast<DeviceManagerError>(e)) {
      case DeviceManagerError::NoHIDFound:     return "No HID found for device";
      default:                                 return "Unknown error";
    }
  }
};

inline const DeviceManagerErrorCategory deviceManagerErrorCategory{};

inline std::error_code make_error_code(DeviceManagerError e) {
  return { static_cast<int>(e), deviceManagerErrorCategory };
}

template <>
struct std::is_error_code_enum<DeviceManagerError> : true_type {};

class DeviceManager {
public:
  DeviceManager();
  ~DeviceManager();

  std::expected<std::vector<InputDevice>, std::error_code> scan();
  std::expected<void, std::pair<std::error_code, std::string>> populateMetadata(std::vector<InputDevice> &input_devices);
  std::map<std::string, std::vector<std::unique_ptr<InputDevice>>> groupByHid(std::vector<InputDevice> &input_devices);
};
