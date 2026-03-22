# WZ Virtual Gamepad

A production-quality Windows application that converts keyboard and mouse input into a virtual Xbox 360 controller (XInput-compatible via ViGEmBus). Specifically optimized for **Call of Duty: Warzone** with aim-assist-eligible controller input.

## Overview

WZ Virtual Gamepad intercepts keyboard and mouse input using the Windows Raw Input API and translates it to virtual Xbox 360 controller signals via ViGEmBus. Games see a real Xbox 360 controller, enabling aim-assist in titles like Warzone that provide it to controller players.

**The kernel driver (ViGEmBus) does NOT read or modify any game process memory.** It only emulates a USB controller at the Plug-and-Play bus level.

## Features

- **Zero-latency input capture** via Windows Raw Input API
- **Virtual Xbox 360 controller** output through ViGEmBus
- **Warzone-optimized** sensitivity curves and key bindings pre-configured
- **Analog stick emulation** from WASD with configurable ramp-up
- **Mouse → right stick** conversion with smoothing, jitter filter, auto-center
- **S-curve sensitivity** for natural aim feel
- **Layer system** (up to 4 shift-layers with different bindings)
- **Macro engine** with pre-built Warzone macros (slide-cancel, bunny-hop, tactical sprint)
- **Profile system** with auto-switching on game focus
- **System tray** icon and OSD overlay
- **Hot-key** mode toggle (F9), profile cycle (F10), emergency stop (F12)

## Quick Start

1. Install [ViGEmBus driver](https://github.com/nefarius/ViGEmBus/releases/latest)
2. Run `installer\install.bat` as Administrator
3. Launch `wz_gamepad.exe`
4. Start Warzone — the virtual controller is detected automatically
5. Press **F9** to toggle gamepad emulation on/off

## Documentation

| Document | Description |
|---|---|
| [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) | Technical architecture and threading model |
| [docs/USER_GUIDE.md](docs/USER_GUIDE.md) | Installation, configuration, and usage |
| [docs/BUILDING.md](docs/BUILDING.md) | Build instructions (MSVC + CMake) |
| [docs/DRIVER_SAFETY.md](docs/DRIVER_SAFETY.md) | Confirmation: no game memory access |
| [docs/WARZONE_TUNING.md](docs/WARZONE_TUNING.md) | Warzone-specific sensitivity tuning |

## Default Key Bindings (Warzone)

| Input | Gamepad Output |
|---|---|
| WASD | Left Stick |
| Mouse Move | Right Stick |
| Left Mouse Button | RT (Fire) |
| Right Mouse Button | LT (ADS) |
| Space | A (Jump) |
| C | B (Crouch/Slide) |
| Left Shift | LS Click (Sprint) |
| Left Ctrl | RS Click (Melee) |
| R | X (Reload) |
| F | Y (Use) |
| G | LB (Tactical) |
| Q | RB (Lethal) |
| Tab | Back (Map) |
| Escape | Start (Menu) |
| Mouse Wheel | D-Pad Up/Down |
| 1 / 2 | D-Pad Left/Right |
| F9 | Toggle Gamepad Mode |
| F10 | Cycle Profiles |
| F12 | Emergency Stop |

## Architecture

```
User-Mode (Ring 3):
  ┌──────────────┐   ┌──────────────┐   ┌──────────────┐
  │  GUI / Tray  │   │ Input Engine │   │  Profile Mgr │
  │  (Win32)     │◄──┤  (Raw Input  │   │  (JSON-based │
  │  OSD overlay │   │   hooks)     │   │   profiles)  │
  └──────────────┘   └──────┬───────┘   └──────┬───────┘
                     ┌──────▼───────┐          │
                     │ Mapping Core │◄─────────┘
                     │ (curves,     │
                     │  dead zones, │
                     │  macros,     │
                     │  layers)     │
                     └──────┬───────┘
                     ┌──────▼───────┐
                     │ ViGEmClient  │ ← Virtual Xbox 360 output
                     └──────┬───────┘
                            │ IOCTL
Kernel-Mode (Ring 0):       │
  ┌─────────────────────────▼───┐
  │  ViGEmBus Driver            │
  │  (emulates Xbox 360 on PnP) │
  │  ⛔ NO game memory access    │
  └─────────────────────────────┘
```

## Building

See [docs/BUILDING.md](docs/BUILDING.md) for full instructions.

**Requirements:**
- Windows 10/11 x64
- Visual Studio 2022 with C++ workload
- CMake 3.20+
- ViGEmBus SDK (header bundled, driver installed separately)

```bat
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

## License

MIT License — see [LICENSE](LICENSE)
