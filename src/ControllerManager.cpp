#include "ControllerManager.hpp"

using std::vector;
using std::unique_ptr;
using std::error_code;
using std::expected;
using std::unexpected;

ControllerManager::ControllerManager(vector<unique_ptr<Controller>> ctrls) : ctrls_(move(ctrls)), connected_(false) {

}

ControllerManager::~ControllerManager() {

}

expected<void, error_code> ControllerManager::connect() {
  for (auto &ctrl : ctrls_) {
    auto conn = ctrl->connect();
    if (!conn) return unexpected(conn.error());
  }
  connected_ = true;
  return {};
}
