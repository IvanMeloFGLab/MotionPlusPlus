#include "DeviceConnection.hpp"

DeviceConnection::DeviceConnection(const InputDevice &device) : device_(device){

  fd_ = open(device_.path.string().c_str(), O_RDONLY);

  if (fd_ < 0) {
    throw runtime_error("Error opening device.");
  }

  dev_ = nullptr;
  rc_ = libevdev_new_from_fd(fd_, &dev_);

  if (rc_ < 0) {
    close(fd_);
    throw runtime_error("Error generating libevdev: " + string(strerror(-rc_)));
  }

}

DeviceConnection::~DeviceConnection() {
  if (dev_) libevdev_free(dev_);
  if (fd_ >= 0) close(fd_);
}

expected<input_event, string> DeviceConnection::read() {
  input_event ev;

  rc_ = libevdev_next_event(dev_, LIBEVDEV_READ_FLAG_NORMAL, &ev);

  if (rc_ == LIBEVDEV_READ_STATUS_SUCCESS) {
    return ev;
  }
  else if (rc_ == LIBEVDEV_READ_STATUS_SYNC) {
    // dropped events, need to sync
    while (rc_ == LIBEVDEV_READ_STATUS_SYNC) {
      rc_ = libevdev_next_event(dev_, LIBEVDEV_READ_FLAG_SYNC, &ev);
    }
    return ev;
  }
  else if (rc_ == -EAGAIN) {
    return unexpected("No event available.");
  }
  else {
    return unexpected(strerror(-rc_));
  }

}
