#include "DeviceManager.hpp"

using std::println;
using std::string;
using std::filesystem::directory_iterator;
using std::filesystem::filesystem_error;
using std::vector;
using std::expected;
using std::unexpected;
using std::error_code;
using std::generic_category;

DeviceManager::DeviceManager() {
}

DeviceManager::~DeviceManager() {

}

expected<vector<InputDevice>, error_code> DeviceManager::scan() {
  vector<InputDevice> input_devices;
  error_code ec;

  for (const auto& file : directory_iterator("/dev/input", ec)) {
    if (ec) return unexpected(ec);

    if (file.path().filename().string().starts_with("event")) {
      input_devices.emplace_back(file.path());
    }
  }

  return input_devices;
}

expected<void, error_code> DeviceManager::populateMetadata(vector<InputDevice> &input_devices) {
  for(auto &in_d : input_devices) {
    int fd = open(in_d.path.string().c_str(), O_RDONLY);

    if (fd < 0) return unexpected(error_code(errno, generic_category()));

    libevdev *dev = nullptr;
    int rc = libevdev_new_from_fd(fd, &dev);

    if (rc < 0) {
      close(fd);
      return unexpected(error_code(-rc, generic_category()));
    }

    in_d.name = libevdev_get_name(dev);
    in_d.vendor = libevdev_get_id_vendor(dev);
    in_d.product = libevdev_get_id_product(dev);
    in_d.bus = libevdev_get_id_bustype(dev);

    auto phys = libevdev_get_phys(dev);
    auto uniq = libevdev_get_uniq(dev);
    in_d.phys = phys ? phys : "";
    in_d.uniq = uniq ? uniq : "";

    libevdev_free(dev);
    close(fd);
  }

  return {};
}
