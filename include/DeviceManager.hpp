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

using std::println;
using std::string;
using std::filesystem::directory_iterator;
using std::filesystem::filesystem_error;
using std::vector;
using std::expected;
using std::unexpected;
using std::error_code;

struct InputDevice {
  std::filesystem::path path;
  string name;
  int vendor;
  int product;
  int bus;
  string phys;
  string uniq;

  InputDevice(std::filesystem::path path) : path(path), name(""), vendor(0), product(0), bus(0), phys(""), uniq("") {}
};

template<>
struct std::formatter<InputDevice> : std::formatter<string> {
  auto format(const InputDevice &id, format_context &ctx) const {
    return std::formatter<string>::format(std::format("Name: {}, Vendor: {}, Product: {}, Bus: {}, Phys: {}, Uniq: {}",
                                                 id.name, id.vendor, id.product, id.bus, id.phys, id.uniq), ctx);
  }
};

class DeviceManager {
public:
  DeviceManager();
  ~DeviceManager();

  expected<vector<InputDevice>, string> scan();
  bool populateMetadata(vector<InputDevice> &input_devices);
};
