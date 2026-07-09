# LibMotionPlusPlus

> A modern C++ library for turning motion controllers into customizable input devices on Linux.

LibMotionPlusPlus is an open-source C++ library focused on bringing advanced support for motion-based controllers such as the Nintendo Wii Remote, Wii MotionPlus, Sony PS Move, and future devices to Linux.

It handles device discovery, hotplug/disconnect handling, and decoding raw controller state — buttons, motion, extensions — reliably and without needing root access. It intentionally does not map input to actions or emulate devices; see [Project Direction](#project-direction) for where that lives.

## Project Direction

LibMotionPlusPlus is a **C++ library**, its scope is : **device discovery, hotplug/disconnect handling, and the controller abstraction layer** — reading raw controller state (buttons, motion, extensions) reliably. It does not map input to actions, emulate devices, or provide any user-facing configuration; that's left to the projects built on top of it.

Three separate projects make up the overall ecosystem:

* **`LibMotionPlusPlus`** *(this repository)* — the core library. Discovers controllers, manages their lifecycle, and exposes raw decoded input. No mapping, no virtual devices, no configuration — just the controllers.
* **`MotionPlusPlus`** *(daemon/application, planned)* — a desktop daemon built on top of the library, responsible for mapping controller input to actions, creating and managing virtual input devices (mouse/keyboard/gamepad emulation), and user-facing profile configuration.
* **`ros2_motionplusplus`** *(planned)* — a ROS 2 package built on top of the library, focused on motion-controller data collection, sensor fusion/pose estimation, and RViz visualization support for robotics use cases.

A small demo executable showing basic usage of the library itself is included under [`example/`](example/).

## Project Goals

* Native Linux support
* Low-latency input processing
* Modular and extensible architecture, packaged as a reusable library
* Hotplug handling (connect/disconnect, partial extension loss)
* Support for multiple controller families

## Planned Controller Support

* Nintendo Wii Remote
* Wii MotionPlus
* Wii Nunchuk
* Sony PS Move
* Additional HID motion controllers in the future

## Current Status

Current milestone:

* Library-oriented CMake build system (`find_package`-able, versioned, exported targets)
* C++23 codebase
* Device discovery with runtime hotplug/disconnect handling
* Metadata extraction using libevdev
* udev-based permission setup — no root required for normal use
* Wii Remote input decoding (buttons, accelerometer, MotionPlus, IR) with LED and battery support

The project is under active development and is **not yet ready for daily use**. The public API is not yet stable (pre-1.0) and may change between minor versions.

## Dependencies

Current dependencies:

* C++23 compatible compiler
* CMake ≥ 3.20
* libevdev
* pkg-config

## Building

```bash
git clone https://github.com/IvanMeloFGLab/LibMotionPlusPlus.git
cd LibMotionPlusPlus
mkdir build && cd build
cmake .. -DMOTIONPLUSPLUS_BUILD_EXAMPLES=ON
cmake --build .
```

To install the library system-wide (headers, compiled library, and CMake package config for `find_package`):

```bash
sudo cmake --install .
```

### Using it as a library in your own CMake project

Once installed:

```cmake
find_package(LibMotionPlusPlus REQUIRED)
target_link_libraries(your_target PRIVATE LibMotionPlusPlus::motionplusplus)
```

### Device permissions (no `sudo` required)

Accessing a Wii Remote's input, LED, and battery interfaces normally requires elevated privileges. MotionPlusPlus ships a udev rule set to avoid this.

**Option 1 — let CMake install it:**

```bash
cmake .. -DMOTIONPLUSPLUS_INSTALL_UDEV_RULES=ON
sudo cmake --install .
```

**Option 2 — install manually:**

```bash
sudo cp udev/99-motionplusplus.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
sudo usermod -aG input $USER
```

Then **log out and back in** (group membership is applied at login), and physically disconnect/reconnect your controller so the new rule is applied to it. After that, running the demo:

```bash
./example/motionplusplus_demo
```

## License

This project is licensed under the **GNU General Public License v3.0**. See [`LICENSE`](LICENSE) for the full text.
