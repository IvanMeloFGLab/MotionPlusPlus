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
        keyboard,
        rel_mouse,
        abs_mouse,
        none
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
        InputType getType() const;

        std::expected<void, std::error_code> open();
        std::expected<void, std::error_code> setKey(uint16_t key, bool state);
        std::expected<void, std::error_code> moveRel(uint16_t code, int32_t delta);
        std::expected<void, std::error_code> moveAbs(uint16_t code, int32_t value);
        std::expected<void, std::error_code> sync();

    private:
        InputType type_;
        bool opened_;
        int fd_;
        std::string name_;

        std::unordered_map<uint16_t, bool> keys_;
        std::unordered_map<uint16_t, int32_t> rels_;
        std::unordered_map<uint16_t, int32_t> abss_;
    };

}
