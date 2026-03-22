# WZ Virtual Gamepad — Architecture

## Overview

WZ Virtual Gamepad is a user-mode Windows application that intercepts keyboard and mouse input and emits virtual Xbox 360 controller (XInput) signals via the ViGEmBus kernel driver. It is specifically optimized for Call of Duty: Warzone.

```
┌─────────────────────────────────────────────────────────────────────┐
│                         User-Mode (Ring 3)                          │
│                                                                     │
│  ┌──────────────┐  ┌─────────────┐  ┌──────────────┐  ┌─────────┐  │
│  │  main.cpp    │  │ InputEngine │  │ MappingEngine│  │ Profile │  │
│  │  (WinMain)   │  │ (Raw Input) │  │ + Layers +   │  │ Manager │  │
│  │  AppController│  │ WM_INPUT    │  │ MacroEngine  │  │ JSON I/O│  │
│  └──────┬───────┘  └──────┬──────┘  └──────┬───────┘  └────┬────┘  │
│         │                 │                 │               │       │
│         └─────────────────▼─────────────────▼───────────────▼──┐   │
│                      AxisProcessor                              │   │
│               (curves, dead zones, smoothing, jitter)          │   │
│                                │                               │   │
│                   ┌────────────▼────────────┐                  │   │
│                   │   VirtualGamepad        │                  │   │
│                   │   (ViGEmClient wrapper) │                  │   │
│                   └────────────┬────────────┘                  │   │
│                                │ IOCTL                         │   │
└────────────────────────────────│───────────────────────────────┘   │
                                 │                                    │
          ┌──────────────────────▼──────────────────────┐            │
          │           Kernel-Mode (Ring 0)              │            │
          │                                             │            │
          │   ViGEmBus.sys (KMDF driver)                │            │
          │   Emulates Xbox 360 HID device on PnP bus   │            │
          │                                             │            │
          │   ⛔ Does NOT read game memory               │            │
          │   ⛔ Does NOT inject code into processes     │            │
          └─────────────────────────────────────────────┘
```

---

## Component Details

### 1. Input Capture — `InputEngine`

**File:** `src/core/input_engine.cpp/h`

Uses the Windows **Raw Input API** to receive keyboard and mouse events.

- Registers for `RIDEV_INPUTSINK` so input is received even when the application is not in focus.
- Processes `WM_INPUT` messages in the application message loop.
- Emits `InputEvent` structs (key up/down, mouse delta X/Y, mouse buttons, scroll wheel).
- Targets **< 0.5 ms** additional latency from hardware event to application receipt.

```cpp
RAWINPUTDEVICE rid[2];
rid[0].usUsagePage = 0x01; rid[0].usUsage = 0x06; // Keyboard
rid[1].usUsagePage = 0x01; rid[1].usUsage = 0x02; // Mouse
rid[0].dwFlags = RIDEV_INPUTSINK;
rid[1].dwFlags = RIDEV_INPUTSINK;
RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE));
```

---

### 2. Axis Processor — `AxisProcessor`

**File:** `src/core/axis_processor.cpp/h`

Applies all mathematical transformations to raw mouse deltas before they become right-stick values.

| Stage | Algorithm |
|-------|-----------|
| Anti-jitter | Discard mouse movements below `threshold` pixels |
| Sensitivity | Scale by `sensitivityX / sensitivityY` |
| S-Curve | `output = x² × (3 - 2x)` (smoothstep, mapped to signed range) |
| Dead zone | Inner: remap `[dz_inner, 1.0] → [0, 1]`; outer: clamp at `dz_outer` |
| EMA Smoothing | `smoothed = α × current + (1-α) × previous` |
| Auto-center | Exponential decay to 0 when no mouse input (configurable decay time) |
| Diagonal norm | Clamp `(x, y)` vector magnitude to unit circle |

The WASD → left stick processor uses a **ramp-up timer** per direction key to produce analog-feeling movement (gradual increase from 0 to full deflection over `ramp_up_ms` milliseconds).

---

### 3. Mapping Engine — `MappingEngine`

**File:** `src/core/mapping_engine.cpp/h`

Converts the current key/button state into an `XUSB_REPORT`.

- Reads key bindings from `MappingConfig` (loaded from active profile).
- Supports mapping any key/mouse button to: gamepad button, left trigger (LT), right trigger (RT).
- Handles scroll wheel → D-Pad Up/Down.
- Delegates left/right stick values to `AxisProcessor`.

---

### 4. Layer Manager — `LayerManager`

**File:** `src/core/layer_manager.cpp/h`

Provides up to 4 simultaneous shift-layers. Each layer can override key bindings and apply a sensitivity multiplier. Layers are stacked: higher-index layers override lower-index bindings.

```
Layer 0 (base) ← always active, loaded from profile
Layer 1 (ADS)  ← active when RMB held; sens_mult = 0.6
Layer 2–3      ← user-defined
```

---

### 5. Macro Engine — `MacroEngine`

**File:** `src/core/macro_engine.cpp/h`

Executes macro sequences in a dedicated background thread. Each macro is a list of `MacroStep` structs containing:
- **Button** — the gamepad button/trigger to press
- **Action** — `press` or `release`
- **Delay** — milliseconds to wait after this step

Pre-built Warzone macros:
- **Slide Cancel**: B → wait 50ms → B → wait 50ms → A
- **Tactical Sprint**: LS → wait 80ms → LS
- **Bunny Hop**: A → wait 150ms → repeat while held

---

### 6. Profile Manager — `ProfileManager`

**File:** `src/profile/profile_manager.cpp/h`

Manages JSON profile files using `nlohmann/json`. Provides:
- Load/save profiles to `%APPDATA%\WZVirtualGamepad\profiles\`
- Hot-reload on file change
- CRUD operations: create, list, activate, delete

---

### 7. Auto Switch — `AutoSwitch`

**File:** `src/profile/auto_switch.cpp/h`

Monitors the foreground window process name using `GetForegroundWindow()` + `GetWindowThreadProcessId()` + `QueryFullProcessImageName()`. When the foreground process matches a profile's `target_game` field, that profile is automatically activated.

---

### 8. Virtual Gamepad — `VirtualGamepad`

**File:** `src/core/virtual_gamepad.cpp/h`

Thin wrapper around the ViGEmClient C API:

```
vigem_alloc()
vigem_connect()
vigem_target_x360_alloc()
vigem_target_add()
→ loop: vigem_target_x360_update(report)
vigem_target_remove()
vigem_target_free()
vigem_disconnect()
vigem_free()
```

Vibration feedback is received via `XUSB_VIBRATION_NOTIFICATION_CALLBACK` and can be passed through to the physical keyboard's LED or logged.

---

### 9. OSD Overlay — `OsdOverlay`

**File:** `src/app/osd_overlay.cpp/h`

A layered, click-through, always-on-top window (`WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST`) drawn with GDI. Displays current profile name, active layer, and emulation mode. Auto-hides after 3 seconds.

---

### 10. Tray Icon — `TrayIcon`

**File:** `src/app/tray_icon.cpp/h`

Uses `Shell_NotifyIcon()` to manage a system tray icon. Right-click context menu provides:
- Switch profile
- Toggle emulation on/off
- Show/hide OSD
- Open settings
- Exit

---

## Threading Model

```
Main Thread
├─ Message loop (GetMessage / DispatchMessage)
├─ WM_INPUT handling → InputEngine
└─ XUSB_REPORT update → VirtualGamepad (synchronous, < 1ms)

Macro Thread (std::thread)
└─ MacroEngine::WorkerThread → sleeps between steps

Auto-Switch Thread (std::thread, 500ms polling)
└─ AutoSwitch::MonitorThread → GetForegroundWindow()
```

---

## Data Flow

```
Physical KB/Mouse
      │  USB HID interrupt
      ▼
Windows HID stack
      │  WM_INPUT message
      ▼
InputEngine::ProcessRawInput()
      │  InputEvent
      ▼
AppController::OnInput()
      ├── LayerManager::GetEffectiveConfig()
      ├── MacroEngine::Tick()
      ├── MappingEngine::ApplyKeyState()      → buttons, triggers
      ├── AxisProcessor::ProcessWasd()        → sThumbLX, sThumbLY
      └── AxisProcessor::ProcessMouseDelta()  → sThumbRX, sThumbRY
              │  XUSB_REPORT
              ▼
      VirtualGamepad::Update()
              │  vigem_target_x360_update() IOCTL
              ▼
      ViGEmBus.sys kernel driver
              │  HID report
              ▼
      Game (via XInput API)
```

---

## Latency Budget

| Stage | Target |
|-------|--------|
| HID → WM_INPUT | ~0.2 ms |
| Input processing + axis math | ~0.1 ms |
| vigem IOCTL round-trip | ~0.5 ms |
| **Total additional latency** | **< 1 ms** |

XInput polling rate in most games: 125–1000 Hz (8–1 ms). The mapper updates at the same rate as input events arrive, which is effectively the mouse polling rate (125–8000 Hz).

---

## Security Design

- **No process injection** — the application never calls `OpenProcess`, `WriteProcessMemory`, or `CreateRemoteThread` on any game process.
- **No memory reads** — the application does not read memory from any other process.
- **No kernel driver modifications** — the application uses ViGEmBus as-is; it does not modify its behavior.
- **Raw Input only** — input is captured at the OS message level, not via driver hooking.
- **Standard PnP device** — the virtual controller appears as a standard USB device on a virtual PnP bus.
