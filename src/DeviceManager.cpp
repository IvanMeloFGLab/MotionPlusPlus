#include "DeviceManager.hpp"

DeviceManager::DeviceManager() {
}

DeviceManager::~DeviceManager() {

}

expected<vector<InputDevice>, string> DeviceManager::scan() {
  vector<InputDevice> input_devices;
  error_code ec;

  for (const auto& file : directory_iterator("/dev/input", ec)) {
    if (ec) return unexpected(ec.message());

    if (file.path().filename().string().starts_with("event")) {
      input_devices.emplace_back(file.path());
    }
  }

  return input_devices;
}

bool DeviceManager::populateMetadata(vector<InputDevice> &input_devices) {
  for(auto &in_d : input_devices) {
    int fd = open(in_d.path.string().c_str(), O_RDONLY);

    if (fd < 0) {
      println("Error opening device: {}", strerror(errno));
      return false;
    }

    libevdev *dev = nullptr;
    int rc = libevdev_new_from_fd(fd, &dev);

    if (rc < 0) {
      println("Error generating libevdev: {}", strerror(-rc));
      close(fd);
      return false;
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

  return true;
}
