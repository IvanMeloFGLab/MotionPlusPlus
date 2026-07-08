#include "DeviceConnection.hpp"

using std::runtime_error;
using std::expected;
using std::unexpected;
using std::string;
using std::error_code;
using std::generic_category;

DeviceConnection::DeviceConnection(const InputDevice &device, int fd, libevdev *dev) : device_(device), fd_(fd), dev_(dev) {

}

DeviceConnection::DeviceConnection(DeviceConnection&& other) : device_(other.device_), fd_(other.fd_), dev_(other.dev_) {
  other.fd_ = -1;
  other.dev_ = nullptr;
}

DeviceConnection::~DeviceConnection() {
  if (dev_) libevdev_free(dev_);
  if (fd_ >= 0) close(fd_);
}

expected<DeviceConnection, error_code> DeviceConnection::connect(const InputDevice &device) {
  int fd = open(device.path.string().c_str(), O_RDWR | O_NONBLOCK);

  if (fd < 0) return unexpected(error_code(errno, generic_category()));

  libevdev *dev = nullptr;
  int rc = libevdev_new_from_fd(fd, &dev);

  if (rc < 0) {
    close(fd);
    return unexpected(error_code(-rc, generic_category()));
  }

  return DeviceConnection(device, fd, dev);
}

expected<input_event, error_code> DeviceConnection::read() {
  input_event ev;

  int rc = libevdev_next_event(dev_, LIBEVDEV_READ_FLAG_NORMAL, &ev);

  if (rc == LIBEVDEV_READ_STATUS_SUCCESS) {
    return ev;
  }
  else if (rc == LIBEVDEV_READ_STATUS_SYNC) {
    // dropped events, need to sync
    while (rc == LIBEVDEV_READ_STATUS_SYNC) {
      rc = libevdev_next_event(dev_, LIBEVDEV_READ_FLAG_SYNC, &ev);
    }
    return ev;
  } else {
    return unexpected(error_code(-rc, generic_category()));
  }
}

expected<int, error_code> DeviceConnection::uploadEffect(ff_effect &effect) {
  if (ioctl(fd_, EVIOCSFF, &effect) < 0) return unexpected(error_code(errno, generic_category()));
  return effect.id;     // kernel fills in the id
}

expected<void, error_code> DeviceConnection::playEffect(int effect_id) {
  input_event ev{};
  ev.type = EV_FF;
  ev.code = effect_id;
  ev.value = 1;         // play once
  if (write(fd_, &ev, sizeof(ev)) < 0) return unexpected(error_code(errno, generic_category()));
  return {};
}

expected<void, error_code> DeviceConnection::stopEffect(int effect_id) {
  input_event ev{};
  ev.type = EV_FF;
  ev.code = effect_id;
  ev.value = 0;
  if (write(fd_, &ev, sizeof(ev)) < 0) return unexpected(error_code(errno, generic_category()));
  return {};
}

int DeviceConnection::getFd() {
  return fd_;
}

const string DeviceConnection::getDeviceName() {
  return device_.name;
}

const InputDevice &DeviceConnection::getDevice() const {
  return device_;
}
