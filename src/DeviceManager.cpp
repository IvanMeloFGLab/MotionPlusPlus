#include "libmotionplusplus/DeviceManager.hpp"

using std::println;
using std::string;
using std::filesystem::directory_iterator;
using std::filesystem::filesystem_error;
using std::vector;
using std::expected;
using std::unexpected;
using std::error_code;
using std::generic_category;
using std::filesystem::canonical;
using std::pair;
using std::make_pair;
using std::map;
using std::unique_ptr;
using std::make_unique;

using namespace motionplusplus;

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

expected<void, pair<error_code, string>> DeviceManager::populateMetadata(vector<InputDevice> &input_devices) {
  for(auto &in_d : input_devices) {
    int fd = open(in_d.path.string().c_str(), O_RDONLY);

    if (fd < 0) return unexpected(make_pair(error_code(errno, generic_category()), in_d.path.string()));

    libevdev *dev = nullptr;
    int rc = libevdev_new_from_fd(fd, &dev);

    if (rc < 0) {
      close(fd);
      return unexpected(make_pair(error_code(-rc, generic_category()), in_d.path.string()));
    }

    in_d.name = libevdev_get_name(dev);
    in_d.vendor = libevdev_get_id_vendor(dev);
    in_d.product = libevdev_get_id_product(dev);
    in_d.bus = libevdev_get_id_bustype(dev);

    auto phys = libevdev_get_phys(dev);
    auto uniq = libevdev_get_uniq(dev);
    in_d.phys = phys ? phys : "";
    in_d.uniq = uniq ? uniq : "";

    string real_path = canonical("/sys/class" + in_d.path.string().substr(4));

    auto last = real_path.find("/input/");
    if (last == string::npos) last = real_path.find("/sound/");
    if (last == string::npos) return unexpected(make_pair(DeviceManagerError::NoHIDFound, in_d.name));

    auto first = real_path.substr(0, last).rfind("/");
    if (first == string::npos) return unexpected(make_pair(DeviceManagerError::NoHIDFound, in_d.name));

    in_d.hid = real_path.substr(first+1, last-(first+1));

    libevdev_free(dev);
    close(fd);
  }

  return {};
}

map<string, vector<unique_ptr<InputDevice>>> DeviceManager::groupByHid(vector<InputDevice> &input_devices) {
  map<string, vector<unique_ptr<InputDevice>>> grouped_devices;
  for(auto &in_d : input_devices) {
    grouped_devices[in_d.hid].emplace_back(make_unique<InputDevice>(in_d));
  }
  return grouped_devices;
}
