# WZ Virtual Gamepad — Driver Safety Statement

## Executive Summary

WZ Virtual Gamepad **does not read, write, or monitor the memory of any game process**. It does not inject code into other processes. It does not bypass or interfere with anti-cheat systems. It operates exclusively through documented, public Windows APIs.

---

## What the Application Does

### User-Mode Application (`wz_gamepad.exe`)

| Action | API Used | Purpose |
|--------|----------|---------|
| Capture keyboard/mouse input | `RegisterRawInputDevices` + `WM_INPUT` | Receive input events from the OS message queue |
| Detect foreground window | `GetForegroundWindow()`, `GetWindowThreadProcessId()` | Auto-switch profiles when game starts |
| Read process name | `QueryFullProcessImageName()` | Match process name to profile (e.g., `cod.exe`) |
| Emit virtual controller | `ViGEmClient` → `vigem_target_x360_update()` | Send gamepad state to kernel driver |
| System tray / OSD | `Shell_NotifyIcon()`, GDI | User interface |
| Register hotkeys | `RegisterHotKey()` | Global keyboard shortcuts |

### Kernel-Mode Driver (`ViGEmBus.sys`)

ViGEmBus is an open-source, WHQL-eligible KMDF driver maintained by [Nefarius Software Solutions](https://github.com/nefarius/ViGEmBus).

| Capability | Status |
|------------|--------|
| Read other process memory | ❌ Never |
| Write other process memory | ❌ Never |
| Create remote threads | ❌ Never |
| Hook system calls (SSDT) | ❌ Never |
| Access game files on disk | ❌ Never |
| Communicate with game process | ❌ Never |
| Emulate USB HID device | ✅ Primary function |
| Receive IOCTL from user-mode app | ✅ From `wz_gamepad.exe` only |

---

## Memory Access Proof

The application source code can be verified to contain **zero** calls to:
- `ReadProcessMemory`
- `WriteProcessMemory`
- `VirtualAllocEx`
- `CreateRemoteThread`
- `NtReadVirtualMemory`
- `OpenProcess` with `PROCESS_VM_READ` or `PROCESS_VM_WRITE`

The only `OpenProcess` call in the codebase is in `src/util/process_monitor.cpp`, using `PROCESS_QUERY_LIMITED_INFORMATION` **only** to read the process image path — not memory.

---

## Input Capture Method

The application uses the **Raw Input API** (`WM_INPUT`), **not**:
- Kernel-level keyboard/mouse hooking
- `SetWindowsHookEx` with `WH_KEYBOARD_LL` or `WH_MOUSE_LL` (low-level hooks detectable by anti-cheat)
- Direct I/O with keyboard/mouse device drivers
- Filter drivers

Raw Input is a standard Windows messaging mechanism. Every application that receives keyboard/mouse input uses it under the hood.

---

## Anti-Cheat Compatibility

### Ricochet Anti-Cheat (Warzone / Call of Duty)
- Ricochet operates as a kernel-mode driver that monitors for unauthorized memory access and code injection.
- WZ Virtual Gamepad does not perform any of the activities Ricochet monitors for.
- The virtual Xbox 360 controller appears at the `XInput` API level — the same level as any physical controller.

### Easy Anti-Cheat / BattlEye
- Both EAC and BattlEye check for process injection, memory manipulation, and driver signatures.
- ViGEmBus is a signed KMDF driver with a valid EV certificate — it passes driver signature checks.
- The mapper application performs no injection.

**Note:** Using any third-party software with online games is at the user's risk. This tool uses only legitimate Windows APIs and a signed kernel driver, similar to commercial tools like DS4Windows and reWASD that are widely used without bans.

---

## Open Source Verification

The complete source code is available in this repository. You can:
1. Review every file for memory access operations.
2. Build from source and verify the binary matches expected behavior.
3. Run the application under a kernel debugger to observe its system calls.

---

## ViGEmBus Driver Signing

ViGEmBus is signed with an **Extended Validation (EV) code signing certificate**, which means:
- Windows Driver Signature Enforcement (DSE) is satisfied without disabling Secure Boot.
- The driver can be installed on any Windows 10/11 system without special modes.
- The signature chain can be verified in Device Manager → Nefarius Virtual Gamepad Emulation Bus → Properties → Driver Details.
