#include "WiiMote.hpp"

using std::vector;
using std::unique_ptr;
using std::shared_ptr;
using std::move;

WiiMote::WiiMote(shared_ptr<DeviceManager> dm, int ctrl_id, vector<std::unique_ptr<InputDevice>> devs) : Controller(dm, ctrl_id, move(devs)) {

}

WiiMote::~WiiMote() {

}
