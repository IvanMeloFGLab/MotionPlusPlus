# Changelog

All notable changes to this project will be documented in this file.

---

## [0.3.0] - 2026-07-06

### Added

- Generic `Controller` abstraction.
- `ControllerManager` responsible for discovering and managing connected controllers.
- Automatic controller discovery pipeline independent from device scanning.
- `Wiimote` implementation built on top of the generic controller interface.
- Aggregation of all Wiimote input interfaces into a single controller object.
- Live state decoding for:
  - Buttons
  - Accelerometer
  - MotionPlus gyroscope
  - IR camera
- Controller update API allowing applications to process all controller events through a single manager.

### Changed

- Separated low-level Linux device management from controller logic.
- Device discovery now produces physical HID devices which are later interpreted by controller-specific discovery routines.
- `DeviceConnection` now behaves as a lightweight RAII wrapper around a single evdev endpoint.
- Improved ownership model using `std::unique_ptr` for devices and controllers.
- Continued migration toward modern C++23 (`std::expected`, move semantics, RAII).

### Verified

- Original Nintendo Wii Remote with MotionPlus.
- Simultaneous decoding of:
  - Button events
  - Accelerometer data
  - MotionPlus gyroscope
  - IR sensor coordinates
- Correct grouping of the four Wiimote event interfaces into one logical controller.

### Known limitations

- Controller output features (LEDs, rumble and speaker) are not yet implemented.
