#include "WiiMote.hpp"

using std::vector;
using std::string;
using std::unique_ptr;
using std::shared_ptr;
using std::move;
using std::pair;
using std::make_pair;
using std::unordered_map;

WiiMote::WiiMote(shared_ptr<DeviceManager> dm, int ctrl_id, vector<std::unique_ptr<InputDevice>> devs) : Controller(dm, ctrl_id, move(devs)) {

}

WiiMote::WiiMote(WiiMote&& other) : Controller(move(other)) {

}

WiiMote::~WiiMote() {

}

pair<vector<unique_ptr<Controller>>, int> WiiMote::discover(shared_ptr<DeviceManager> dm, int ctrl_id_off,
                                                         unordered_map<string, vector<unique_ptr<InputDevice>>> &grps) {
  vector<unique_ptr<Controller>> ctrls;
  int i = ctrl_id_off;

  for (auto &grp : grps) {
    if (grp.first.find("0005:057E:0306") != string::npos) { //Bluetooth:Nintendo:Wiimote
      ctrls.emplace_back(unique_ptr<WiiMote>(new WiiMote(dm, i, move(grp.second))));
      i++;
    }
  }

  return make_pair(move(ctrls), i);
}
