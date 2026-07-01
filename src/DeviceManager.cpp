#include "DeviceManager.hpp"

DeviceManager::DeviceManager() {
}

DeviceManager::~DeviceManager() {

}


vector<InputDevice> DeviceManager::scan() {
  vector<InputDevice> input_devices;

  try {
    for (const auto& file : directory_iterator("/dev/input")) {
      if (file.path().filename().string().starts_with("event")) {
        input_devices.emplace_back(file.path());
      }
    }
  }
  catch (const filesystem_error& e) {
    std::cerr << e.what() << '\n';
  }

  return input_devices;
}

bool DeviceManager::populateMetadata(vector<InputDevice> &input_devices) {
  for(auto &in_d : input_devices) {
    int fd = open(in_d.path.string().c_str(), O_RDONLY);

    if (fd < 0) {
      std::cerr << "Error opening input event. Do you forget sudo?" << std::endl;
      return false;
    }

    libevdev *dev = nullptr;
    int rc = libevdev_new_from_fd(fd, &dev);

    if (rc < 0) {
      std::cerr << "Error generating libevdev: " << strerror(-rc) << std::endl;
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
