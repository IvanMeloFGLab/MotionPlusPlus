# Changelog

All notable changes to this project will be documented in this file.

---

## [0.4.0] - 2026-07-09

### Changed — Project renamed

- Repository renamed from `MotionPlusPlus` to **`LibMotionPlusPlus`**, reflecting its shift to a shared library. The compiled artifact and namespace remain `motionplusplus` (unaffected); the project/package name used by `find_package()` and the exported CMake namespace changed to `LibMotionPlusPlus`.

### Changed — Project direction

- **The project is shifting from a standalone daemon to a reusable C++ library.** `LibMotionPlusPlus` now handles device discovery, hotplug/disconnect handling, and the controller abstraction layer only — no input mapping, virtual device emulation, or configuration. That scope moves to two separate downstream projects:
  - `MotionPlusPlus` *(planned)* — desktop daemon: input mapping, virtual device emulation, profiles.
  - `ros2_motionplusplus` *(planned)* — ROS 2 package: motion data collection, sensor fusion, RViz support.
- All public headers moved under `include/libmotionplusplus/` and wrapped in the `motionplusplus` namespace to avoid symbol/header collisions for consumers.
- The former `main.cpp` entry point moved to `example/` and now builds as a separate demo executable (`motionplusplus_demo`) linked against the library the same way an external consumer would.
- Migrated from C++20 to **C++23**, now used throughout (`std::expected`, `std::print`).

### Added — Library packaging

- CMake target `libmotionplusplus` (aliased as `LibMotionPlusPlus::libmotionplusplus`) with proper `PUBLIC`/`PRIVATE` dependency propagation, so consumers linking against it automatically receive the correct include paths and dependencies (`libevdev` via `pkg-config`) without manual configuration.
- `install()` rules and generated `LibMotionPlusPlusConfig.cmake` / `LibMotionPlusPlusConfigVersion.cmake`, making the library discoverable via `find_package(LibMotionPlusPlus)` from any other CMake project.
- `BUILD_SHARED_LIBS` and `MOTIONPLUSPLUS_BUILD_EXAMPLES` CMake options.
- Optional, opt-in installation of the udev rule set via `INSTALL_UDEV_RULES` (off by default — see below).

### Added — udev rules (no more `sudo` required)

- Shipped `udev/99-motionplusplus.rules`, granting the running user access to Wii Remote input, LED, and battery interfaces without elevated privileges, provided the user is a member of the `input` group.
- Rule set covers three previously-separate permission domains that required different fixes:
  - `/dev/input/event*` nodes for the Wiimote and its sub-devices (accelerometer, IR, Motion Plus, Nunchuk), matched by HID vendor/product ID.
  - LED brightness control (`/sys/class/leds/*:blue:p*`), which has no `/dev` node and required a `RUN+=` rule to `chmod`/`chgrp` the sysfs file directly, since standard `MODE=`/`GROUP=` rule keys only apply to device nodes.
  - Battery level reporting, confirmed to already be world-readable by kernel default and requiring no rule.
- Documented manual installation steps (copy rule, reload udev, add user to `input` group, reconnect device) in the README for consumers not using the CMake install option.

### Added — Hotplug and disconnect handling

- `ControllerManager` now detects and handles device connection/disconnection at runtime instead of only at startup:
  - Periodic lightweight polling (`lightScan()`, wall-clock interval-based) detects new devices without the cost of full metadata discovery on every tick.
  - New devices are merged into existing controllers rather than triggering full rediscovery, with deduplication by device path to prevent duplicate entries when a controller's sub-devices (e.g. Motion Plus, Nunchuk) register asynchronously after the core device.
  - Distinguishes **partial disconnection** (an extension like Motion Plus or Nunchuk drops while the core controller remains usable) from **full disconnection** (the entire controller is gone), handled via `Controller::onLostFd`.
  - Device loss is now detected via both `POLLHUP`/`POLLERR` on `poll()` and via `ENODEV`/`EIO`/`ENOTCONN` surfacing from an in-flight `read()`, both routed through the same cleanup path.
  - Controller IDs are assigned incrementally and never reused by a *different* physical controller while still tracked, avoiding identity swaps between currently-connected controllers when another one disconnects.

### Fixed

- `DeviceManager::populateMetadata` no longer aborts an entire device scan when a single node fails to open due to a transient hotplug race (`EACCES`) — the affected node is skipped and picked up on a subsequent scan instead.
- Fixed a use-after-free: the Wiimote LED startup animation previously ran on a detached thread capturing `this`, which could outlive the `WiiMote` object on disconnect. The thread is now owned and joined in the destructor before the object is torn down.
- Fixed several iterator-invalidation bugs where disconnect handling (`ctrls_.erase`, `fds_` rebuild) occurred while still iterating the containers being modified.
- Fixed LED animation occasionally not running on fast reconnects, caused by the LED's sysfs classdev not yet being fully attached at the moment `WiiMote`'s constructor ran; the animation now waits briefly before its first attempt.

### Known limitations

- Controller output feature (speaker) is not yet implemented.
- Requires a recent compiler (GCC 14+ or Clang 17+) due to `std::print` usage, which may not be the system default on many current distros.

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
