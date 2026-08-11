#pragma once

#include <fcntl.h>
#include <unistd.h>
#include <expected>
#include <system_error>
#include <cstring>

// Virtual controller
#include <linux/uinput.h>
#include <linux/input-event-codes.h>   // KEY_A, BTN_LEFT, etc.


namespace motionplusplus {
    enum class InputType {
        keyboard
    };

    class VirtualController {
    public:
        VirtualController(InputType type);
        ~VirtualController();

        VirtualController(const VirtualController&) = delete;
        VirtualController& operator=(const VirtualController&) = delete;
        VirtualController(VirtualController&&) = default;
        VirtualController& operator=(VirtualController&&) = default;

        std::expected<void, std::error_code> open();
        std::expected<void, std::error_code> setKey(bool state);
        std::expected<void, std::error_code> sync();

    private:
        InputType type_;
        bool opened_;
        int fd_;
    };

}
