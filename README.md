# MotionPlusPlus

Modern C++23 library for Nintendo Wii Remote support on Linux.

MotionPlusPlus is a Linux-native controller framework focused on the Nintendo Wii Remote family. The library provides direct access to the Wiimote's individual sensors and capabilities through a modern C++ API.

The project is designed with extensibility in mind, allowing additional HID-based controllers to be added without changing the core architecture.

---

## Current features

- Linux evdev backend
- Automatic discovery of HID devices
- Physical device grouping
- Generic controller abstraction
- Wiimote controller implementation
- Reading:
  - Buttons
  - Accelerometer
  - MotionPlus gyroscope
  - IR camera coordinates

---

## Current architecture

```
                DeviceManager
                      │
        scans /dev/input/event*
                      │
              InputDevice objects
                      │
          grouped by physical HID
                      │
             Controller discovery
                      │
        ┌─────────────┴─────────────┐
        │                           │
    WiiMote                  Future controllers
        │
  DeviceConnection(s)
        │
     Linux evdev
```

---

## Project goals

MotionPlusPlus is intended to become a complete Linux framework for motion controllers, including:

- Wiimote
- MotionPlus
- Sony PS Move support (planned)

Long-term goals include:

- LED control
- Rumble
- Speaker support
- Motion tracking
- VR experimentation
- ROS 2 integration through the separate **ros2_motionplusplus** project

---

## Requirements

- Linux
- C++23
- CMake ≥ 3.20
- libevdev

---

## Build

```bash
mkdir build
cd build

cmake ..
make
```

---

## Current status

The project is in active development.

Implemented:

- ✔ Device discovery
- ✔ Controller discovery
- ✔ Wiimote state decoding
- ✔ Multi-controller polling

In progress:

- ⏳ Output features (LEDs, rumble)
- ⏳ Extension devices
