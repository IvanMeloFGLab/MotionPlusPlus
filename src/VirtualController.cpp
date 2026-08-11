#include "motionplusplus/VirtualController.hpp"

using std::expected;
using std::unexpected;
using std::error_code;
using std::generic_category;

using namespace motionplusplus;

VirtualController::VirtualController(InputType type) : type_(type), opened_(false) {}

VirtualController::~VirtualController() {
    if (opened_) {
        ioctl(fd_, UI_DEV_DESTROY, NULL);
        close(fd_);
    }
}

expected<void, error_code> VirtualController::open() {
    fd_ = ::open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd_ < 0) return unexpected(error_code(errno, generic_category()));
    opened_ = true;

    if (ioctl(fd_, UI_SET_EVBIT, EV_KEY) < 0) return unexpected(error_code(errno, generic_category()));   // key events
    if (ioctl(fd_, UI_SET_EVBIT, EV_SYN) < 0) return unexpected(error_code(errno, generic_category()));

    // Buttons
    if (ioctl(fd_, UI_SET_KEYBIT, KEY_A) < 0) return unexpected(error_code(errno, generic_category()));

    // Creation
    struct uinput_setup usetup = {};
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor  = 0xF61B;
    usetup.id.product = 0x0001;
    strcpy(usetup.name, "MotionPlusPlus Virtual Device");

    if (ioctl(fd_, UI_DEV_SETUP, &usetup) < 0) return unexpected(error_code(errno, generic_category()));
    if (ioctl(fd_, UI_DEV_CREATE, NULL) < 0) return unexpected(error_code(errno, generic_category()));

    return {};
}

expected<void, error_code> VirtualController::setKey(bool state) {
    struct input_event ev = {};
    ev.type = EV_KEY;
    ev.code = KEY_A;
    ev.value = state;
    if (write(fd_, &ev, sizeof(ev)) != sizeof(ev)) return unexpected(error_code(errno, generic_category()));
    return sync();
}

expected<void, error_code> VirtualController::sync() {
    struct input_event syn = {};
    syn.type = EV_SYN;
    syn.code = SYN_REPORT;
    syn.value = 0;
    if (write(fd_, &syn, sizeof(syn)) != sizeof(syn)) return unexpected(error_code(errno, generic_category()));
    return {};
}
