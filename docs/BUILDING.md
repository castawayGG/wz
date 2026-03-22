# WZ Virtual Gamepad — Build Instructions

## Prerequisites

### Required Tools
- **Visual Studio 2022** (Community, Professional, or Enterprise)
  - Workload: "Desktop development with C++"
  - Individual components: "C++ CMake tools for Windows", "Windows 10/11 SDK"
- **CMake 3.20+** (bundled with VS2022, or download from cmake.org)
- **Git** for FetchContent dependencies

### Required Runtime
- **ViGEmBus** kernel driver (for testing) — [Download here](https://github.com/nefarius/ViGEmBus/releases/latest)
- **ViGEmClient SDK** — only needed to link; the header is bundled in `include/vigem/Client.h`

---

## Quick Build (Visual Studio Developer Command Prompt)

```cmd
git clone https://github.com/castawayGG/wz.git
cd wz

:: Configure (Debug)
cmake -B build -G "Visual Studio 17 2022" -A x64 ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DBUILD_TESTING=ON

:: Build
cmake --build build --config Release --parallel

:: Output: build\Release\wz_gamepad.exe
```

---

## Linking ViGEmClient

The application header (`include/vigem/Client.h`) is bundled. You need the import library (`ViGEmClient.lib`) and DLL (`ViGEmClient.dll`) from the ViGEmClient SDK.

### Option A: Use the CMake option (recommended)

Download the ViGEmClient SDK from [nefarius/ViGEmClient](https://github.com/nefarius/ViGEmClient/releases):

```cmd
cmake -B build -G "Visual Studio 17 2022" -A x64 ^
    -DWZ_VIGEM_LIB="C:\path\to\ViGEmClient\x64\ViGEmClient.lib"
```

### Option B: System-wide installation

Install ViGEmClient SDK system-wide and CMake will find it automatically via the default search paths.

### Option C: Build without ViGEmClient (headers only)

The project compiles without `ViGEmClient.lib` — just remove the `vigem_*` calls or define `WZ_VIGEM_STUB` to build without linking. This is useful for running unit tests.

---

## Build Configurations

| Configuration | Description |
|---------------|-------------|
| `Release` | Optimized, no debug symbols. Use for deployment. |
| `Debug` | Debug symbols, assertions enabled. |
| `RelWithDebInfo` | Optimized with debug symbols. Recommended for profiling. |

---

## Running Unit Tests

```cmd
cmake -B build -G "Visual Studio 17 2022" -A x64 -DBUILD_TESTING=ON
cmake --build build --config Release --target wz_tests
ctest --test-dir build --build-config Release --output-on-failure
```

The tests do not require ViGEmBus to be installed; they test the axis math, mapping, macro, and profile logic in isolation.

---

## CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_TESTING` | `OFF` | Build unit tests |
| `WZ_VIGEM_LIB` | `""` | Path to `ViGEmClient.lib` |

---

## Project Structure

```
wz/
├── CMakeLists.txt           # Root CMake
├── external/
│   └── CMakeLists.txt       # FetchContent: nlohmann/json
├── include/
│   └── vigem/
│       └── Client.h         # ViGEmClient SDK header (bundled)
├── src/                     # Application source
├── tests/                   # Unit tests
├── profiles/                # Default JSON profiles
├── docs/                    # Documentation
└── installer/               # Installation scripts
```

---

## Continuous Integration

The project uses GitHub Actions. To replicate the CI build locally:

```cmd
cmake -B build -G "Visual Studio 17 2022" -A x64 ^
    -DBUILD_TESTING=ON ^
    -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
ctest --test-dir build --build-config Release
```

---

## Troubleshooting Build Issues

### `nlohmann/json` not found
CMake FetchContent downloads it automatically. Ensure you have internet access or pre-download to `external/json`.

### `ViGEmClient.lib` not found
Set `-DWZ_VIGEM_LIB=<path>` or comment out the ViGEm linking in `CMakeLists.txt` for a header-only build.

### MSVC Compiler version errors
Minimum supported: MSVC 19.29 (VS2019 16.11+). Update Visual Studio to the latest version.

### Windows SDK not found
Install "Windows 10 SDK (10.0.19041.0)" or newer via the Visual Studio Installer.
