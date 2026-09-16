#include "motionplusplus/VirtualController.hpp"
#include <libmotionplusplus/DeviceManager.hpp>

using std::expected;
using std::unexpected;
using std::error_code;
using std::generic_category;
using std::string;

using namespace motionplusplus;

VirtualController::VirtualController(InputType type) : type_(type), opened_(false) {
    switch (type_) {
        case InputType::keyboard: {
            for (uint16_t key = KEY_RESERVED; key <= KEY_F24; ++key) keys_[key] = false;
            name_ = "keyboard";
            break;
        } case InputType::mouse: {
            for (uint16_t key = BTN_MOUSE; key <= BTN_TASK; ++key) keys_[key] = false;
            for (uint16_t rel = REL_X; rel <= REL_MISC; ++rel) rels_[rel] = 0;
            name_ = "mouse";
            break;
        } case InputType::none: {
            name_ = "not_valid";
            break;
        }
    }
}

VirtualController::~VirtualController() {
    if (opened_) {
        ioctl(fd_, UI_DEV_DESTROY, NULL);
        close(fd_);
    }
}

VirtualController::VirtualController(VirtualController&& other) noexcept : type_(other.type_), opened_(other.opened_), fd_(other.fd_),
name_(std::move(other.name_)), keys_(std::move(other.keys_)) {
    other.opened_ = false;
    other.fd_ = -1;
}

VirtualController& VirtualController::operator=(VirtualController&& other) noexcept {
    if (this != &other) {
        if (opened_) { ioctl(fd_, UI_DEV_DESTROY, NULL); close(fd_); }
        type_ = other.type_;
        opened_ = other.opened_;
        fd_ = other.fd_;
        name_ = std::move(other.name_);
        keys_ = std::move(other.keys_);
        other.opened_ = false;
        other.fd_ = -1;
    }
    return *this;
}

int VirtualController::getFd() const {
  return fd_;
}

InputType VirtualController::getType() const {
    return type_;
}

expected<void, error_code> VirtualController::open() {
    if (opened_) return {};
    fd_ = ::open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd_ < 0) return unexpected(error_code(errno, generic_category()));
    opened_ = true;

    switch (type_) {
        case InputType::keyboard: {
            if (ioctl(fd_, UI_SET_EVBIT, EV_KEY) < 0) return unexpected(error_code(errno, generic_category()));   // key events
            if (ioctl(fd_, UI_SET_EVBIT, EV_SYN) < 0) return unexpected(error_code(errno, generic_category()));

            // Buttons
            for (const auto key : keys_) {
                if (ioctl(fd_, UI_SET_KEYBIT, key.first) < 0) return unexpected(error_code(errno, generic_category()));
            }

            break;
        } case InputType::mouse: {
            if (ioctl(fd_, UI_SET_EVBIT, EV_KEY) < 0) return unexpected(error_code(errno, generic_category()));   // key events
            if (ioctl(fd_, UI_SET_EVBIT, EV_REL) < 0) return unexpected(error_code(errno, generic_category()));   // relative events
            if (ioctl(fd_, UI_SET_EVBIT, EV_SYN) < 0) return unexpected(error_code(errno, generic_category()));

            // Buttons
            for (const auto key : keys_) {
                if (ioctl(fd_, UI_SET_KEYBIT, key.first) < 0) return unexpected(error_code(errno, generic_category()));
            }

            // Relative axis
            for (const auto rel : rels_) {
                if (ioctl(fd_, UI_SET_RELBIT, rel.first) < 0) return unexpected(error_code(errno, generic_category()));
            }

            break;
        } case InputType::none: {
            return unexpected(error_code(static_cast<int>(std::errc::no_such_file_or_directory), generic_category()));
        }
    }

    // Creation
    struct uinput_setup usetup = {};
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor  = 0xF61B;
    usetup.id.product = 0x0001;
    strcpy(usetup.name, string("MotionPlusPlus Virtual" + name_).c_str());

    if (ioctl(fd_, UI_DEV_SETUP, &usetup) < 0) return unexpected(error_code(errno, generic_category()));
    if (ioctl(fd_, UI_DEV_CREATE, NULL) < 0) return unexpected(error_code(errno, generic_category()));

    return {};
}

expected<void, error_code> VirtualController::setKey(uint16_t key, bool state) {
    if (!keys_.contains(key)) return unexpected(error_code(static_cast<int>(std::errc::no_such_file_or_directory), generic_category()));
    if (keys_[key] == state) return {};
    struct input_event ev = {};
    ev.type = EV_KEY;
    ev.code = key;
    ev.value = state;
    if (write(fd_, &ev, sizeof(ev)) != sizeof(ev)) return unexpected(error_code(errno, generic_category()));
    keys_[key] = state;
    return {};
}

expected<void, error_code> VirtualController::moveRel(uint16_t code, int32_t delta) {
    if (type_ != InputType::mouse) return unexpected(error_code(static_cast<int>(std::errc::operation_not_supported), generic_category()));
    if (!rels_.contains(code)) return unexpected(error_code(static_cast<int>(std::errc::no_such_file_or_directory), generic_category()));
    if (delta == 0) return {};
    struct input_event ev = {};
    ev.type = EV_REL;
    ev.code = code;
    ev.value = delta;
    if (write(fd_, &ev, sizeof(ev)) != sizeof(ev)) return unexpected(error_code(errno, generic_category()));
    rels_[code] = delta;
    return {};
}

expected<void, error_code> VirtualController::sync() {
    struct input_event syn = {};
    syn.type = EV_SYN;
    syn.code = SYN_REPORT;
    syn.value = 0;
    if (write(fd_, &syn, sizeof(syn)) != sizeof(syn)) return unexpected(error_code(errno, generic_category()));
    return {};
}
