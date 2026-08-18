#pragma once

#include <cstdint>
#include <fcntl.h>
#include <unistd.h>
#include <expected>
#include <system_error>
#include <cstring>
#include <unordered_map>

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
        VirtualController(VirtualController&&) noexcept;
        VirtualController& operator=(VirtualController&&) noexcept;

        int getFd() const;

        std::expected<void, std::error_code> open();
        std::expected<void, std::error_code> setKey(uint16_t key, bool state);
        std::expected<void, std::error_code> sync();

    private:
        InputType type_;
        bool opened_;
        int fd_;
        std::string name_;

        std::unordered_map<uint16_t, bool> keys_;
    };

}
