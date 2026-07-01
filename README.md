# Wiimote Linux Research Notes
**Project:** Native Linux Wiimote Integration Daemon (C++)
**Status:** Initial hardware investigation completed

---

# Goal

Develop a native Linux daemon that turns the Nintendo Wii Remote into a first-class Linux input device capable of:

- Desktop mouse
- Keyboard shortcuts
- Steam/Game Mode navigation
- Gesture recognition
- Custom actions
- Automatic profile switching
- Dolphin compatibility

Target platforms:

- Ubuntu (development)
- Bazzite
- SteamOS
- ChimeraOS

---

# Initial Hardware

Tested device:

- Original Nintendo Wii Remote (RVL-CNT-01)
- Official MotionPlus accessory attached

Connected through Bluetooth.

---

# Kernel Support

Linux automatically loads:

```text
hid_wiimote
```

Kernel modules loaded:

```text
hid_wiimote
ff_memless
hid
```

No external Wiimote library was required for basic functionality.

---

# Device Enumeration

The Linux kernel exposes the Wiimote as **four separate evdev devices**.

## event7

```
Nintendo Wii Remote Accelerometer
```

Capabilities

- Accelerometer X
- Accelerometer Y
- Accelerometer Z

Axis:

```
ABS_RX
ABS_RY
ABS_RZ
```

Range:

```
-500 ... 500
```

---

## event8

```
Nintendo Wii Remote IR
```

Capabilities

- IR Camera tracking

Linux exposes four tracked IR points.

Axes:

```
ABS_HAT0X
ABS_HAT0Y

ABS_HAT1X
ABS_HAT1Y

ABS_HAT2X
ABS_HAT2Y

ABS_HAT3X
ABS_HAT3Y
```

Ranges:

```
X : 0 - 1023

Y : 0 - 767
```

This corresponds to **up to four simultaneously tracked infrared blobs**.

No camera image is exposed.

---

## event9

```
Nintendo Wii Remote
```

Capabilities

Buttons

```
D-Pad

A
B

1
2

Home

+
-
```

Linux exposes:

```
EV_KEY
```

Also exposes

```
EV_FF
```

meaning Force Feedback (rumble) is available through the Linux input subsystem.

Handlers:

```
kbd
event9
js0
```

Meaning the Wiimote simultaneously behaves as

- keyboard-like input
- joystick
- evdev input device

---

## event10

```
Nintendo Wii Remote Motion Plus
```

Capabilities

Gyroscope

Axes:

```
ABS_RX
ABS_RY
ABS_RZ
```

Approximate range

```
-16000 ... 16000
```

---

# IR Camera Findings

Very important discovery.

The Wiimote **does not transmit a camera image**.

Instead it contains a PixArt IR camera that performs onboard image processing.

The Wiimote only transmits:

- up to four IR blobs
- X coordinate
- Y coordinate

The image itself is inaccessible through Linux's hid_wiimote driver.

Therefore

```
Camera image
        ❌

Tracked IR coordinates
        ✔
```

---

# Current Linux Architecture

```
Bluetooth
      │
hid_wiimote (Kernel)
      │
 ┌────┼────────┬────────┬────────┐
 │    │        │        │
 │    │        │        │
IR  Buttons  Accel   Motion+
 │    │        │        │
event8 event9 event7 event10
```

---

# Major Discovery

The Linux kernel already separates every Wiimote subsystem.

This means a custom daemon may only need:

```
libevdev
```

instead of a dedicated Wiimote library.

This greatly simplifies the architecture.

---

# Proposed Architecture

```
                WiimoteDaemon

            Device Manager
                   │
        ┌──────────┼──────────┐
        │          │          │
     Buttons       IR     Motion
        │          │          │
        └──────────┼──────────┘
                   │
             Action Engine
                   │
      ┌────────────┼────────────┐
      │            │            │
   Mouse      Keyboard      Gamepad
      │            │            │
          libevdev/uinput
                   │
              Linux Desktop
```

---

# Planned Features

## Input

- Mouse
- Keyboard
- Virtual Gamepad

---

## Pointer

- Wii IR pointer
- MotionPlus pointer
- Hybrid pointer
- Calibration
- Smoothing
- Dead zones

---

## Buttons

Examples

```
A
→ Left Click

B
→ Right Click

Power
→ Suspend

Home
→ Steam

+
→ Volume Up

-
→ Volume Down
```

---

## Gestures

Potential gestures

- Shake
- Flick Left
- Flick Right
- Rotate
- Throw
- Double Shake

---

## Profiles

Automatic profile switching.

Examples

Desktop

```
Mouse
```

Steam

```
Steam shortcuts
```

Kodi

```
Media controls
```

Dolphin

```
Release Wiimote
Allow Dolphin direct access
```

---

# Configuration

Planned TOML configuration.

Example

```toml
[button.power]
action = "system.sleep"

[button.home]
action = "steam"

[button.a]
action = "mouse.left"

[button.b]
action = "mouse.right"

[gesture.shake]
action = "media.playpause"

[ir]
enabled = true
smoothing = 0.80
deadzone = 4
```

---

# Libraries (Tentative)

Required

- libevdev
- libuinput

Recommended

- toml++
- fmt
- spdlog
- sdbus-c++

---

# Open Questions

Still unknown

- LED control
- Battery status
- Speaker support
- Extension hot-plug handling
- Nunchuk support
- Classic Controller support
- Automatic reconnect
- Dolphin handoff implementation

---

# Next Milestone

Create the first C++ application.

Goals

1. Discover all Wiimote event devices.
2. Open them using libevdev.
3. Print button events.
4. Print accelerometer values.
5. Print MotionPlus values.
6. Print IR coordinates.

No mouse output yet.

Only verify communication.

After that:

```
libevdev
        ↓
Action Engine
        ↓
uinput
        ↓
Virtual Mouse
```
