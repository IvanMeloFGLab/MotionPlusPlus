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

Active development. Currently supports:

- Discovering and tracking multiple simultaneously-connected Wii Remotes
- Config-driven button → keyboard key mapping (TOML)
- Creating and managing a real virtual keyboard device per controller via
  `uinput`, so mapped input shows up system-wide like a real keyboard
- Clean lifecycle handling: stuck-key release on disconnect, `SIGTERM`/`SIGINT`
  shutdown with proper virtual device cleanup
- Runs as a systemd **user** service (no root required)

Not yet implemented: accelerometer/gyro/IR mapping, mouse/gamepad virtual
device types, config hot-reload (`SIGHUP`).

## Dependencies

- [LibMotionPlusPlus](https://github.com/IvanMeloFGLab/LibMotionPlusPlus)
  (built and installed, `find_package`-able)
- [toml++](https://github.com/marzer/tomlplusplus) (`pacman -S tomlplusplus`
  on Arch, or see their repo for other distros)
- C++23 compatible compiler
- CMake ≥ 3.20

## Building

```bash
git clone https://github.com/IvanMeloFGLab/MotionPlusPlus.git
cd MotionPlusPlus
mkdir build && cd build
cmake .. -DINSTALL_UDEV_RULES=ON
cmake --build .
sudo cmake --install .
```

Requires `LibMotionPlusPlus` to already be installed system-wide (see its
README for build/install/udev-permission instructions), and your user to be
in the `input` and `uinput` groups (`sudo usermod -aG input,uinput $USER`,
then log out/in).

Installs the binary to `~/.local/bin`, a systemd user unit to
`~/.config/systemd/user/`, and an example config to
`~/.config/motionplusplus/`.

## Configuration

Config lives at `~/.config/motionplusplus/config.toml` (falls back to the
installed example if not present). Example:

```toml
name = "my_config"

[wiimote]
a = "KEY_A"
b = "KEY_B"
up = "KEY_UP"
down = "KEY_DOWN"
left = "KEY_LEFT"
right = "KEY_RIGHT"
plus = "KEY_KPPLUS"
minus = "KEY_KPMINUS"
home = "KEY_HOME"
one = "KEY_1"
two = "KEY_2"
```

Target values are standard Linux key names from
[`input-event-codes.h`](https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h).

## Running

As a systemd user service (recommended):

```bash
systemctl --user enable --now motionplusplus
journalctl --user -u motionplusplus -f   # logs
```

Or directly, for testing:

```bash
./motionplusplus
```

## License

This project is licensed under the **GNU General Public License v3.0**. See [`LICENSE`](LICENSE) for the full text.
