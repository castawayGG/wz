# WZ Virtual Gamepad — Warzone Tuning Guide

## Sensitivity Calibration

### Understanding the Parameters

```json
"sensitivity": {
  "x": 15.0,
  "y": 13.0,
  "ads_multiplier": 0.6,
  "hipfire_boost": 1.0
}
```

| Parameter | Description | Warzone Equivalent |
|-----------|-------------|---------------------|
| `x` | Horizontal sensitivity scale | Horizontal Stick Sensitivity |
| `y` | Vertical sensitivity scale | Vertical Stick Sensitivity |
| `ads_multiplier` | ADS sens as fraction of hipfire | ADS Sensitivity Multiplier |
| `hipfire_boost` | Extra boost in hipfire (normally 1.0) | — |

### Recommended Starting Points by DPI

| Mouse DPI | Suggested `x` | Suggested `y` |
|-----------|---------------|---------------|
| 400 DPI | 30.0 | 27.0 |
| 800 DPI | 15.0 | 13.0 |
| 1600 DPI | 7.5 | 6.5 |
| 3200 DPI | 3.8 | 3.3 |

Start with these values and adjust by ±20% until flick shots feel natural at 50m engagement range.

### Finding Your ADS Multiplier
1. Set `ads_multiplier: 1.0` temporarily.
2. Aim at a distant static target in hipfire.
3. Note the stick position to maintain tracking.
4. Now ADS and attempt the same tracking motion.
5. Adjust `ads_multiplier` until the physical mouse movement required is the same in both.

---

## Curve Types Explained

```json
"curve": {
  "type": "s_curve",
  "steepness": 4.0
}
```

### `linear`
- 1:1 mouse movement to stick movement.
- Best for players who prefer total control.
- No acceleration or deceleration zones.

### `quadratic`
- Small movements → much smaller stick output (precise micro-adjustments).
- Large movements → proportionally larger output.
- Good for snipers.

### `s_curve` (recommended for Warzone)
- Uses smoothstep formula: `f(x) = x² × (3 - 2x)`
- Slow in the center (micro-adjustments) and fast at the edges (flicks).
- `steepness`: 3.0 = gentle S, 6.0 = aggressive S.
- **Default: 4.0** — good balance for all engagement ranges.

### `cubic`
- Similar to quadratic but more aggressive center dead zone.
- Only recommended at very high sensitivity.

### `custom_bezier`
- Define your own curve with two Bezier control points.
- `p1: [0.25, 0.1]` = start of curve
- `p2: [0.75, 0.9]` = end of curve

---

## Dead Zone Tuning

```json
"dead_zone": {
  "inner": 0.05,
  "outer": 0.95,
  "type": "circular"
}
```

| Parameter | Description | Effect |
|-----------|-------------|--------|
| `inner` | Below this value, stick = 0 | Eliminates idle drift |
| `outer` | Above this value, stick = max | Ensures full deflection is reachable |
| `type` | `circular` or `square` | `circular` recommended for natural feel |

### Inner Dead Zone
- **Too low (< 0.03)**: Micro-vibrations register, aim drifts.
- **Too high (> 0.15)**: Center of screen feels "sticky", slow to start moving.
- **Warzone default**: 0.05

### Outer Dead Zone
- **Too low (< 0.85)**: Can't reach full sensitivity — flicks feel weak.
- **Normal range**: 0.90–0.98

---

## Smoothing

```json
"smoothing": {
  "enabled": true,
  "alpha": 0.3
}
```

The smoothing uses an **Exponential Moving Average (EMA)**:
```
smoothed = alpha × current + (1 - alpha) × previous
```

| Alpha | Response | Smoothness | Use Case |
|-------|----------|------------|----------|
| 0.1 | Very slow | Very smooth | Slow, precise sniping |
| 0.3 | Moderate | Smooth | Default Warzone |
| 0.5 | Fast | Some smoothing | Aggressive play |
| 0.8 | Very fast | Minimal | Raw mouse feel |
| 1.0 | Instant | No smoothing | Disabled |

**Lower alpha = more smoothing = more latency.** At 0.3, the 63% step response is ~3 frames at 60fps.

---

## Anti-Jitter

```json
"anti_jitter": {
  "enabled": true,
  "threshold": 2
}
```

Ignores mouse movements of `threshold` pixels or fewer. Prevents tiny hand tremors from moving the aim.

- **threshold: 0** = disabled
- **threshold: 1–2** = recommended for most players
- **threshold: 3–5** = for players with hand tremor issues

---

## Auto-Center (Stick Return)

```json
"auto_center": {
  "enabled": true,
  "decay_ms": 50
}
```

When mouse stops moving, the right stick decays exponentially back to center over `decay_ms` milliseconds. This prevents aim from "drifting" when you stop the mouse.

- **decay_ms: 30** = snap back to center very quickly
- **decay_ms: 50** = default, natural feel
- **decay_ms: 100** = slow return, momentum-like feel

---

## Left Stick (WASD) Tuning

```json
"left_stick": {
  "ramp_up_ms": 30,
  "walk_modifier_key": "LAlt",
  "walk_deflection": 0.5,
  "diagonal_normalize": true
}
```

| Parameter | Description |
|-----------|-------------|
| `ramp_up_ms` | Time to reach full stick deflection after key press |
| `walk_modifier_key` | Hold this key to walk instead of run |
| `walk_deflection` | Stick deflection when walking (0.0–1.0) |
| `diagonal_normalize` | Prevent diagonal movement from exceeding max speed |

### Ramp-Up Timing
- **ramp_up_ms: 0** = instant full deflection (tap-strafing, no analog feel)
- **ramp_up_ms: 30** = default — smooth analog emulation
- **ramp_up_ms: 100** = very gradual, realistic controller feel

---

## Profile-Specific Recommendations

### AR / General Play (`warzone_default`)
```json
"sensitivity": { "x": 15.0, "y": 13.0, "ads_multiplier": 0.6 },
"curve": { "type": "s_curve", "steepness": 4.0 },
"smoothing": { "alpha": 0.3 }
```

### Sniper (`warzone_sniper`)
```json
"sensitivity": { "x": 8.0, "y": 7.0, "ads_multiplier": 0.35 },
"curve": { "type": "s_curve", "steepness": 5.0 },
"smoothing": { "alpha": 0.2 },
"anti_jitter": { "threshold": 3 }
```

### SMG / Aggressive (`warzone_aggressive`)
```json
"sensitivity": { "x": 22.0, "y": 20.0, "ads_multiplier": 0.75 },
"curve": { "type": "quadratic", "steepness": 3.0 },
"smoothing": { "alpha": 0.45 },
"anti_jitter": { "threshold": 1 }
```

---

## Warzone In-Game Controller Settings

For the virtual controller to feel natural in Warzone, set:

| Setting | Recommended Value |
|---------|------------------|
| Aim Assist | Standard (for controller players) |
| Aim Assist Window Size | 6–8 |
| Aim Response Curve Type | **Standard** (mapper applies its own curve) |
| Left Stick Min Input | 0 (mapper handles dead zones) |
| Right Stick Min Input | 0 (mapper handles dead zones) |
| Left Stick Max Input | 99 |
| Right Stick Max Input | 99 |
| Controller Vibration | Optional |

> **Important**: Set Aim Response Curve Type to **Standard** in Warzone because the mapper already applies the s_curve transformation. Using "Dynamic" in-game on top of the mapper's curve would double-curve your input.

---

## Macro Timing for Warzone

### Slide Cancel Timing
The slide cancel macro uses 50ms delays. In Warzone (Modern Warfare II/III and Warzone 2.0/Warzone — both the original and the 2.0 reboot), the slide cancel timing window is approximately 40–80ms. If the macro doesn't work:
1. Increase delay to 60ms: `"delay_ms": 60`
2. Or decrease to 40ms for faster cancel: `"delay_ms": 40`

### Bunny Hop Timing
The 150ms delay between jumps is optimized for Warzone's jump cooldown. Adjust if the game patches this.

### Tactical Sprint Double-Tap
The 80ms window matches Warzone's double-tap detection. If you're already running, a single LS press initiates tactical sprint in some modes.

---

## Latency Testing

To measure your total input-to-output latency:
1. Enable logging in the profile: `"log_latency": true`
2. Check `%APPDATA%\WZVirtualGamepad\wz_gamepad.log` for latency measurements.
3. Target: < 2ms from Raw Input event to ViGEm IOCTL completion.

For competitive play, use a 1000 Hz mouse and keep `smoothing.alpha` ≥ 0.3.
