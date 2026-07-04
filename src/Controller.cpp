#include "Controller.hpp"

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
using std::println;

Controller::Controller(shared_ptr<DeviceManager> dm, int ctrl_id, vector<unique_ptr<InputDevice>> devs) : dm_(dm), ctrl_id_(ctrl_id), devs_(move(devs)) {
  hid_ = devs_.front()->hid;
  connected_ = false;
}

Controller::~Controller() {

}

expected<void, error_code> Controller::connect() {
  for (auto &dev : devs_) {
    auto conn = DeviceConnection::connect(*dev);
    if (!conn) return unexpected(conn.error());
    conns_.emplace_back(move(*conn));
  }
  connected_ = true;
  return {};
}

expected<input_event, error_code> Controller::read(int dev_num) {
  if (!connected_) return unexpected(error_code(ENOTCONN, generic_category()));
  if (conns_.size() >= static_cast<size_t>(dev_num)) {
    return conns_[dev_num].read();
  }
  return unexpected(error_code(ENODEV, generic_category()));
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
