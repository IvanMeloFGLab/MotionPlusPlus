#include "ControllerManager.hpp"

#define it_ctrls for (auto &ctrl : ctrls_)

using std::vector;
using std::unique_ptr;
using std::error_code;
using std::expected;
using std::unexpected;
using std::generic_category;
using std::map;
using std::pair;
using std::string;
using std::make_shared;
using std::unordered_set;
using std::chrono::steady_clock;

using milis = std::chrono::milliseconds;

ControllerManager::ControllerManager() : device_num_(0), scan_interval_(milis(250)), last_scan_(steady_clock::now()), new_controllers_(false) {

}

ControllerManager::~ControllerManager() {

}

expected<vector<pair<int, string>>, error_code> ControllerManager::scan() {
  auto input_devices = lightScan();
  if (!input_devices) return unexpected(input_devices.error());

  auto res = dm_.populateMetadata(*input_devices);
  if (!res) return unexpected(res.error().first);

  auto groups = dm_.groupByHid(*input_devices);

  unordered_set<string> knownHids;
  it_ctrls { knownHids.insert(ctrl.second->getHid()); }
  for (auto it = groups.begin(); it != groups.end(); ) {
    if (knownHids.count(it->first)) {
      it_ctrls {
        if (ctrl.second->getHid() == it->first) {
          ctrl.second->addDevices(move(it->second));
          ctrl.second->setConnected(false);
          new_controllers_ = true;
          break;
        }
      }
      it = groups.erase(it);
    } else ++it;
  }

  vector<int> knowIds;
  it_ctrls { knowIds.push_back(ctrl.second->getId()); }

  auto discovered = WiiMote::discover(make_shared<DeviceManager>(dm_), knowIds, groups);
  for (auto &[id, ctrl] : discovered.first) {
    ctrls_.emplace(id, std::move(ctrl));
    new_controllers_ = true;
  }

  vector<pair<int, string>> tmp;
  for (const auto &ctrl : ctrls_) {
    tmp.emplace_back(ctrl.second->getId(), ctrl.second->getType());
  }
  return tmp;
}

expected<void, error_code> ControllerManager::connect() {
  it_ctrls {
    if (!ctrl.second->isConnected()) {
      auto conn = ctrl.second->connect();
      if (!conn) return unexpected(conn.error());
    }
  }
  return updateFds();
}

expected<void, error_code> ControllerManager::update(milis timeout) {
  if (steady_clock::now() - last_scan_ >= scan_interval_) {
    last_scan_ = steady_clock::now();
    auto input_devices = dm_.scan();
    if (!input_devices) return unexpected(input_devices.error());
    if (static_cast<int>(input_devices->size()) > device_num_) {
      auto scn = scan();
      if (!scn) return unexpected(scn.error());
      if (new_controllers_) {
        auto cn = connect();
        if (!cn) return unexpected(cn.error());
      }
      device_num_ = input_devices->size();
    }
  }

  int ret = poll(fds_.data(), fds_.size(), timeout.count());
  if (ret < 0) return unexpected(error_code(errno, generic_category()));

  for (auto &fd : fds_) {
    it_ctrls {
      if (ctrl.second->isFdFromCtrl(fd.fd)) {
        if (fd.revents & POLLIN) {                                       //data available to read
          auto ev = ctrl.second->read(fd.fd);

          if (!ev) {
            if (ev.error().value() == EAGAIN) continue;
            if (ev.error().value() == ENODEV || ev.error().value() == EIO || ev.error().value() == ENOTCONN) {
              auto dis = disconnect(ctrl, fd.fd);
              if (!dis) return unexpected(dis.error());
            }
            return unexpected(ev.error());
          }

          ctrl.second->update(fd.fd, *ev);
          continue;
        }

        if (fd.revents & POLLHUP){
          auto dis = disconnect(ctrl, fd.fd);
          if (!dis) return unexpected(dis.error());
        }
        if (fd.revents & POLLERR)
          return unexpected(error_code(POLLERR, generic_category()));    //error on fd
      }
    }
  }
  return {};
}

vector<int> ControllerManager::getActiveControllers() {
  vector<int> ctrls_ids;
  it_ctrls {ctrls_ids.push_back(ctrl.first);}
  return ctrls_ids;
}

Controller* ControllerManager::getController(int id) {
  return ctrls_[id].get();
}

expected<void, error_code> ControllerManager::disconnect(pair<const int, unique_ptr<Controller>> &ctrl, int fd) {
  if (ctrl.second->onLostFd(fd)) {
    ctrls_.erase(ctrl.first);
  }
  auto lsc = lightScan();
  if (!lsc) return unexpected(lsc.error());
  auto up = updateFds();
  if (!up) return unexpected(up.error());
  return unexpected(error_code(POLLHUP, generic_category()));    //device disconnected/hung up
}

expected<void, error_code> ControllerManager::updateFds() {
  fds_.clear();
  it_ctrls {
    if (!ctrl.second->isConnected()) return unexpected(error_code(ENOTCONN, generic_category()));
    for (auto &c_fd : ctrl.second->getFds()) {
      fds_.emplace_back(c_fd, POLLIN, 0);
    }
  }
  return {};
}

expected<vector<InputDevice>, error_code> ControllerManager::lightScan() {
  auto input_devices = dm_.scan();
  if (!input_devices) return unexpected(input_devices.error());
  device_num_ = input_devices->size();
  return input_devices;
}

bool ControllerManager::isNewContrllers() {
  bool tmp = new_controllers_;
  new_controllers_ = false;
  return tmp;
}
