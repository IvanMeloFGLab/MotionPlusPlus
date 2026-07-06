#include "ControllerManager.hpp"

#define it_ctrls for (auto &ctrl : ctrls_)

using std::vector;
using std::unique_ptr;
using std::error_code;
using std::expected;
using std::unexpected;
using std::generic_category;
using std::unordered_map;

using milis = std::chrono::milliseconds;

ControllerManager::ControllerManager(unordered_map<int, unique_ptr<Controller>> ctrls) : ctrls_(move(ctrls)) {

}

ControllerManager::~ControllerManager() {

}

expected<void, error_code> ControllerManager::connect() {
  it_ctrls {
    auto conn = ctrl.second->connect();
    if (!conn) return unexpected(conn.error());
  }
  return updateFds();
}

expected<void, error_code> ControllerManager::update(milis timeout) {
  int ret = poll(fds_.data(), fds_.size(), timeout.count());
  if (ret < 0) return unexpected(error_code(errno, generic_category()));

  for (auto &fd : fds_) {
    it_ctrls {
      if (ctrl.second->isFdFromCtrl(fd.fd)) {
        if (fd.revents & POLLIN) {                                       //data available to read
          auto ev = ctrl.second->read(fd.fd);

          if (!ev) {
            if (ev.error().value() == EAGAIN) continue;
            return unexpected(ev.error());
          }

          ctrl.second->update(fd.fd, *ev);
          continue;
        }

        if (fd.revents & POLLHUP)
          return unexpected(error_code(POLLHUP, generic_category()));    //device disconnected/hung up
        if (fd.revents & POLLERR)
          return unexpected(error_code(POLLERR, generic_category()));    //error on fd
      }
    }
  }

  return {};
}

expected<void, error_code> ControllerManager::updateFds() {
  it_ctrls {
    if (!ctrl.second->isConnected()) return unexpected(error_code(ENOTCONN, generic_category()));
    for (auto &c_fd : ctrl.second->getFds()) {
      fds_.emplace_back(c_fd, POLLIN, 0);
    }
  }
  return {};
}
