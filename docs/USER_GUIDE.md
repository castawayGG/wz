# WZ Virtual Gamepad — User Guide

## Introduction

WZ Virtual Gamepad converts your keyboard and mouse into a virtual Xbox 360 controller. Call of Duty: Warzone will see a real controller connected, enabling aim-assist while you continue using KB+M precision.

---

## System Requirements

- Windows 10 or Windows 11 (64-bit)
- [ViGEmBus driver](https://github.com/nefarius/ViGEmBus/releases/latest) installed
- Mouse with polling rate of 125 Hz or higher (1000 Hz recommended)
- Warzone: Modern Warfare II/III or Warzone 2.0/Warzone Mobile (PC)

---

## Installation

1. Download and install [ViGEmBus](https://github.com/nefarius/ViGEmBus/releases/latest) (`ViGEmBus_Setup_x64.exe`).
2. Run `installer\install.bat` as **Administrator** (right-click → Run as administrator).
3. Or run the NSIS installer `WZVirtualGamepad_Setup.exe` if provided.
4. Launch `wz_gamepad.exe` — a tray icon will appear.

---

## Default Key Bindings

| Key / Mouse | Gamepad | Warzone Action |
|-------------|---------|----------------|
| W | Left Stick Up | Move Forward |
| A | Left Stick Left | Move Left |
| S | Left Stick Down | Move Back |
| D | Left Stick Right | Move Right |
| Mouse Move | Right Stick | Aim / Look |
| Left Mouse Button | Right Trigger (RT) | Fire |
| Right Mouse Button | Left Trigger (LT) | ADS |
| Space | A | Jump |
| C | B | Crouch / Slide |
| Left Shift | Left Stick Click (LS) | Sprint / Tactical Sprint |
| Left Ctrl | Right Stick Click (RS) | Melee |
| R | X | Reload / Interact |
| F | Y | Use / Killstreak |
| G | Left Bumper (LB) | Tactical Equipment |
| Q | Right Bumper (RB) | Lethal Equipment |
| Tab | Back / Select | Scoreboard |
| Escape | Start | Pause Menu |
| Scroll Up | D-Pad Up | Weapon Up |
| Scroll Down | D-Pad Down | Weapon Down |
| 1 | D-Pad Left | Weapon Slot 1 |
| 2 | D-Pad Right | Weapon Slot 2 |
| Mouse Button 4 (X1) | Left Bumper (LB) | Ping |
| Mouse Button 5 (X2) | A | Jump / Bunny Hop |
| Left Alt + WASD | Left Stick (half deflection) | Walk (non-sprint) |

---

## Global Hotkeys

| Hotkey | Action |
|--------|--------|
| **F9** | Toggle emulation on/off |
| **F10** | Cycle to next profile |
| **F8** | Toggle OSD overlay |
| **F12** | Emergency stop (releases all buttons) |

---

## Profiles

Profiles are JSON files stored in `%APPDATA%\WZVirtualGamepad\profiles\` (user profiles) and the installation's `profiles\` directory (built-in profiles).

### Built-in Profiles

| Profile | Description | Best For |
|---------|-------------|----------|
| `warzone_default` | Balanced, S-curve, 800 DPI | AR / General play |
| `warzone_sniper` | Low sensitivity, precise | Snipers / Long range |
| `warzone_aggressive` | High sensitivity, fast flicks | SMG / Close range |
| `warzone_vehicle` | Vehicle control layout | Driving / Flying |
| `generic_fps` | General FPS template | Other games |

### Switching Profiles

- **Tray icon** → right-click → select profile from submenu.
- **F10** hotkey to cycle profiles.
- **Automatic** when Warzone (cod.exe) becomes the foreground window.

### Editing Profiles

Open the profile JSON with any text editor. The file format is documented in `docs/WARZONE_TUNING.md`. After saving, the application detects the change and hot-reloads automatically.

---

## Warzone In-Game Settings

For best results, configure Warzone as follows:

**Controller Settings:**
- Controller Vibration: On (optional)
- Aim Assist: **Standard** or **Black Ops** (required for aim-assist to engage)
- Aim Assist Window Size: 6–8
- Aim Response Curve Type: **Standard** (the mapper applies its own curve)

**Mouse & Keyboard Settings:**
- Keep KB+M settings at default (they don't affect the virtual controller)

**Important:** Warzone must detect the virtual controller. Verify in:  
`Settings → Controller → Controller Input → Xbox 360 Controller`

---

## ADS (Aim Down Sights) Layer

When you hold **Right Mouse Button (RMB)**, the mapper automatically activates the ADS layer:
- Sensitivity is reduced by the `ads_multiplier` (default 0.60 = 60% of hipfire sensitivity).
- This matches Warzone's built-in ADS sensitivity behavior.

---

## Macros

### Slide Cancel
- **Trigger:** V key
- **Sequence:** Crouch (B) → 50ms → Crouch (B) → 50ms → Jump (A)
- **Use:** Cancel the momentum of a slide to re-sprint faster.

### Tactical Sprint
- **Trigger:** Left Shift + W (sprint key)
- **Sequence:** Sprint (LS) → 80ms → Sprint (LS) double-tap
- **Use:** Activate full Tactical Sprint instantly.

### Bunny Hop
- **Trigger:** Mouse Button 5 (X2), hold
- **Sequence:** Jump (A) → 150ms → repeat while held
- **Use:** Chain jumps for unpredictable movement.

---

## Troubleshooting

### Virtual controller not detected in Warzone
1. Ensure ViGEmBus is installed: check Device Manager → `Nefarius Virtual Gamepad Emulation Bus`.
2. Verify emulation is ON (tray icon should not show a red X).
3. Restart Warzone after starting the mapper.

### Warzone bans / Anti-cheat
WZ Virtual Gamepad:
- Does NOT read or modify game memory.
- Does NOT inject code into the game process.
- Uses a standard, signed PnP driver (ViGEmBus).
- Appears to Warzone as a normal USB Xbox 360 controller.

The application uses the same technique as commercial tools like DS4Windows and reWASD.

### Input lag
- Ensure mouse polling rate is 1000 Hz.
- Close background applications.
- Reduce `smoothing.alpha` in your profile (lower = faster response, more jitter).
- Disable anti-jitter or reduce threshold.

### Mouse movement feels wrong
- Adjust `sensitivity.x` and `sensitivity.y` in your profile.
- Try changing the curve type to `linear` for 1:1 feel.
- Adjust `dead_zone.inner` if small movements feel unresponsive.

### Keys not working
- Make sure emulation mode is active (F9 toggles it).
- Check the tray icon for current mode.
- Verify key names in your profile match the expected virtual key names.

---

## FAQ

**Q: Will this get me banned in Warzone?**  
A: The tool only emulates a standard controller. It does not touch game memory or bypass anti-cheat. However, using any third-party tool is at your own risk. See `docs/DRIVER_SAFETY.md` for full details.

**Q: Can I use this alongside a real controller?**  
A: Yes. The virtual controller appears as an additional device. Use F9 to toggle the virtual controller on/off.

**Q: Does it work with other games?**  
A: Yes. Any game that supports Xbox 360 controllers will work. Use the `generic_fps` profile as a starting point.

**Q: How do I change sensitivity?**  
A: Edit the `sensitivity` section in your profile JSON, or see `docs/WARZONE_TUNING.md` for a detailed guide.
