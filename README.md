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
 
**v1.0** — first stable release. Currently supports:
 
- Discovering and tracking multiple simultaneously-connected Wii Remotes,
  each with its own independent config profile (or falling back to a shared
  default profile)
- Config-driven mapping (TOML), per controller:
  - **Buttons → keyboard, mouse buttons, or both** (`map_to = "keyboard"`,
    `"mouse"`, or `"mouse+keyboard"` — button targets can mix `KEY_*` and
    `BTN_*` names freely and are routed to the right virtual device
    automatically)
  - **Accelerometer → keyboard**, via a per-axis normalized threshold
    (`0.0`–`1.0`) and trigger direction (`+`, `-`, or `+/-`)
  - **IR camera → mouse pointer**, in either relative (velocity-style) or
    absolute (direction-style) mode — see *Known limitations* below
- Real virtual input devices via `uinput` (keyboard, relative mouse, absolute
  mouse), created and cleaned up correctly per controller, including on
  disconnect and daemon shutdown
- Clean lifecycle handling: stuck-key release on disconnect, `SIGTERM`/`SIGINT`
  shutdown with proper virtual device cleanup
- Runs as a systemd **user** service (no root required for day-to-day use)
Not yet implemented: gamepad virtual device type, config hot-reload
(`SIGHUP`), accelerometer trigger hysteresis (see *Known limitations*).
 
## Known limitations
 
- **IR absolute-pointer mode is experimental.** The relative/absolute mode
  switch and the underlying `uinput` plumbing work, but the coordinate math
  (screen mapping, distance/sensitivity handling) is placeholder — real
  pointing-angle-based math is planned as a research follow-up in the
  `ros2_motionplusplus` project, then ported back here.
- **Accelerometer triggers have no hysteresis.** A reading sitting near the
  configured threshold can chatter (rapid trigger/release) rather than
  switching cleanly once. Workaround for now: pick trigger values comfortably
  away from your controller's resting values.
- **Config changes require a daemon restart.** No `SIGHUP`/hot-reload support
  yet (`systemctl --user restart motionplusplus`).
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
cmake ..
cmake --build .
cmake --install .
```
 
No `sudo` needed for the steps above — everything installs under your own
home directory (`~/.local/bin`, `~/.config/...`). Builds in `Release` mode by
default (warnings shown, not fatal); pass `-DCMAKE_BUILD_TYPE=Debug` during
development if you want warnings treated as errors (`-Werror`).
 
The udev rule that grants your user permission to create virtual devices and
read the controller needs `root` and is installed **separately, once**, as
its own component:
 
```bash
sudo cmake --install . --component udev_rules
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
installed example if not present). Controllers are configured as an array of
profiles — each optionally pinned to a specific controller `ID` (as assigned
by `LibMotionPlusPlus`), with a profile lacking an `ID` acting as the default
for any controller without its own dedicated entry:
 
```toml
name = "my_config"
 
[[wiimote]]
# no ID — this is the default profile
 
[wiimote.buttons]
map_to = "mouse+keyboard"
a = "BTN_LEFT"
b = "BTN_RIGHT"
right = "KEY_RIGHT"
left = "KEY_LEFT"
up = "KEY_UP"
down = "KEY_DOWN"
 
[wiimote.ir]
map_to = "mouse"
mode = "relative"       # "relative" or "absolute" (experimental, see Known limitations)
sensitivity = 2.5
 
[[wiimote]]
ID = 2                  # this profile only applies to controller 2
 
[wiimote.buttons]
map_to = "keyboard"
a = "KEY_W"
b = "KEY_E"
plus = "KEY_KPPLUS"
minus = "KEY_KPMINUS"
home = "KEY_HOME"
one = "KEY_1"
two = "KEY_2"
 
[wiimote.accel]
map_to = "keyboard"
x.key = "KEY_X"
x.trigger = 0.5              # normalized 0.0-1.0 against the sensor's declared range
x.trigger_direction = "+"    # "+", "-", or "+/-"
y.key = "KEY_Y"
y.trigger = 0.4
z.key = "KEY_Z"
z.trigger = 0.65
```
 
Key/button target values are standard Linux event codes from
[`input-event-codes.h`](https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h)
(`KEY_*` and `BTN_*`).
 
## Running
 
As a systemd user service (recommended):
 
```bash
systemctl --user daemon-reload
systemctl --user enable --now motionplusplus
journalctl --user -u motionplusplus -f   # logs
```
 
Or directly, for testing:
 
```bash
./motionplusplus
```
 
## License

This project is licensed under the **GNU General Public License v3.0**. See [`LICENSE`](LICENSE) for the full text.
