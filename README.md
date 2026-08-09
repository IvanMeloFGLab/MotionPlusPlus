# MotionPlusPlus

> Desktop daemon that turns motion controllers into real input devices on Linux.

MotionPlusPlus is the daemon/application layer built on top of
[LibMotionPlusPlus](https://github.com/IvanMeloFGLab/LibMotionPlusPlus). It reads
decoded controller state from the library and is responsible for mapping that
input to actions, creating and managing virtual input devices (mouse/keyboard/
gamepad emulation), and user-facing profile configuration.

It is part of a small ecosystem:

- **`LibMotionPlusPlus`** — the core library. Device discovery, hotplug handling,
  raw controller state. No mapping, no virtual devices.
- **`MotionPlusPlus`** *(this repository)* — the daemon. Mapping, virtual devices,
  profiles.
- **`ros2_motionplusplus`** *(planned)* — ROS 2 package for robotics use cases.

## Status

Very early / active development. Nothing here is stable yet. Currently just
wraps `LibMotionPlusPlus::ControllerManager` to discover and print connected
controllers — no input mapping or virtual device emulation yet.

## Dependencies

- [LibMotionPlusPlus](https://github.com/IvanMeloFGLab/LibMotionPlusPlus)
  (built and installed, `find_package`-able)
- C++23 compatible compiler
- CMake ≥ 3.20

## Building

```bash
git clone https://github.com/IvanMeloFGLab/MotionPlusPlus.git
cd MotionPlusPlus
mkdir build && cd build
cmake ..
cmake --build .
```

Requires `LibMotionPlusPlus` to already be installed system-wide (see its
README for build/install/udev-permission instructions).

Run it:

```bash
./motionplusplus
```

## License

This project is licensed under the **GNU General Public License v3.0**. See [`LICENSE`](LICENSE) for the full text.
