# Changelog

All notable changes to this project will be documented in this file.

## [0.1.0] - 2026-07-03

### Added

- Initial MotionPlusPlus project structure using modern C++23 and CMake.
- `InputDevice` abstraction representing Linux evdev input endpoints.
- `DeviceManager` for:
  - Scanning `/dev/input/event*`.
  - Reading device metadata through libevdev.
  - Extracting HID identifiers from sysfs.
  - Grouping event devices belonging to the same physical HID device.
- `DeviceConnection` RAII wrapper for safely opening, managing and reading Linux input devices.
- Factory-based connection API using `std::expected` instead of exceptions.
- `Wiimote` controller abstraction that aggregates all event interfaces belonging to a single Nintendo Wii Remote.
- Live event reading from Wiimote interfaces.

### Changed

- Adopted `std::expected` for recoverable errors across the project.
- Improved ownership semantics using RAII and move-only resource management.
- Added formatted output for discovered input devices.
- Switched from endpoint-oriented discovery to physical HID device grouping.

### Verified

- Successfully detects and groups original Nintendo Wii Remotes with MotionPlus.
- Successfully groups composite HID devices exposing multiple event interfaces.
- Verified on:
  - Nintendo Wii Remote
  - USB keyboards
  - USB mice
  - Composite HID receivers

### Technical Notes

MotionPlusPlus now distinguishes between:

- Linux input endpoints (`/dev/input/eventX`)
- Physical HID devices
- High-level controller abstractions (`Wiimote`)

This architecture provides a generic foundation for adding support for additional HID-based controllers in the future while keeping device discovery independent from controller-specific implementations.
