#include "WiiMote.hpp"

using std::vector;
using std::string;
using std::unique_ptr;
using std::shared_ptr;
using std::move;
using std::pair;
using std::make_pair;
using std::unordered_map;
using std::expected;
using std::unexpected;
using std::error_code;
using std::generic_category;

WiiMote::WiiMote(shared_ptr<DeviceManager> dm, int ctrl_id, vector<std::unique_ptr<InputDevice>> devs) : Controller(dm, ctrl_id, move(devs)) {

}

WiiMote::WiiMote(WiiMote&& other) : Controller(move(other)) {

}

WiiMote::~WiiMote() {

}

pair<unordered_map<int, unique_ptr<Controller>>, int> WiiMote::discover(shared_ptr<DeviceManager> dm, int ctrl_id_off,
                                                         unordered_map<string, vector<unique_ptr<InputDevice>>> &grps) {
  unordered_map<int, unique_ptr<Controller>> ctrls;
  int i = ctrl_id_off;

  for (auto &grp : grps) {
    if (grp.first.find("0005:057E:0306") != string::npos) { //Bluetooth:Nintendo:Wiimote
      ctrls.emplace(i, unique_ptr<WiiMote>(new WiiMote(dm, i, move(grp.second))));
      i++;
    }
  }

  return make_pair(move(ctrls), i);
}

void WiiMote::update(int fd, input_event ev) {
  for (auto &conn : conns_) {
    if (conn.getFd() == fd) {
      if (ev.type == EV_SYN) return;
      if (ev.type == EV_KEY) {
        switch (ev.code) {
          case 304: {
            btns_.a = ev.value;
            break;
          } case 305: {
            btns_.b = ev.value;
            break;
          } case 106: {
            btns_.right = ev.value;
            break;
          } case 105: {
            btns_.left = ev.value;
            break;
          } case 103: {
            btns_.up = ev.value;
            break;
          } case 108: {
            btns_.down = ev.value;
            break;
          } case 407: {
            btns_.plus = ev.value;
            break;
          } case 316: {
            btns_.home = ev.value;
            break;
          } case 412: {
            btns_.minus = ev.value;
            break;
          } case 257: {
            btns_.one = ev.value;
            break;
          } case 258: {
            btns_.two = ev.value;
            break;
          }
        }
        return;
      }
    }
  }
}
