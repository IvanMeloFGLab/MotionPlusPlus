#include "libmotionplusplus/Controller.hpp"

using std::string;
using std::vector;
using std::unique_ptr;
using std::shared_ptr;
using std::expected;
using std::unexpected;
using std::error_code;
using std::make_unique;
using std::generic_category;
using std::move;
using std::ifstream;

using namespace motionplusplus;

Controller::Controller(shared_ptr<DeviceManager> dm, int ctrl_id, vector<unique_ptr<InputDevice>> devs) : dm_(dm), ctrl_id_(ctrl_id), devs_(move(devs)) {
  hid_ = devs_.front()->hid;
  connected_ = false;
}

Controller::~Controller() {

}

expected<void, error_code> Controller::connect() {
  for (auto &dev : devs_) {
    bool skip = false;
    for (auto &conn : conns_) {
      if (dev->name == conn.getDeviceName()) { skip = true; break; }
    }
    if (!skip) {
      auto conn = DeviceConnection::connect(*dev);
      if (!conn) return unexpected(conn.error());
      conns_.emplace_back(move(*conn));
    }
  }
  connected_ = true;
  return {};
}

expected<input_event, error_code> Controller::read(int dev_num) {
  if (!connected_) return unexpected(error_code(ENOTCONN, generic_category()));

  for (auto &conn : conns_) {
    if (conn.getFd() == dev_num) return conn.read();
  }

  return unexpected(error_code(ENODEV, generic_category()));
}

void Controller::removeDeviceFor(const DeviceConnection& conn) {
  devs_.erase(std::remove_if(devs_.begin(), devs_.end(), [&](const unique_ptr<InputDevice>& d) {
    return d.get() == &conn.getDevice();
  }), devs_.end());
}

void Controller::addDevices(vector<unique_ptr<InputDevice>> devs) {
  for (auto &dev : devs) {
    bool exists = false;
    for (auto &existing : devs_) {
      if (existing->path == dev->path) { exists = true; break; }
    }
    if (!exists) devs_.push_back(std::move(dev));
  }
}

vector<string> Controller::getDevicesNames() const {
  vector<string> tmp;
  for (const auto &dev : devs_) {
    tmp.push_back(dev->name);
  }
  return tmp;
}

int Controller::getId() const {
  return ctrl_id_;
}

string Controller::getHid() const {
  return hid_;
}

expected<string, error_code> Controller::getMac() const{
  ifstream uevent("/sys/bus/hid/devices/" + hid_ + "/uevent");
  if (!uevent) return unexpected(error_code(errno, generic_category()));

  string line;
  while (getline(uevent, line)) {
    if (line.find("HID_UNIQ=") != string::npos) {
      return line.substr(line.find("=") + 1);
    }
  }
  return unexpected(error_code(ENOENT, generic_category()));
}

vector<int> Controller::getFds() {
  vector<int> fds;
  for (auto &conn : conns_) {
    fds.push_back(conn.getFd());
  }
  return fds;
}

void Controller::setConnected(bool conn) {
  connected_ = conn;
}

bool Controller::isConnected() {
  return connected_;
}

bool Controller::isFdFromCtrl(int &fd) {
  for (auto &conn : conns_) {
    if (conn.getFd() == fd) return true;
  }
  return false;
}

std::string Controller::getType() const {
  return type_;
}

