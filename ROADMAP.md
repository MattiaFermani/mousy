# 🗺️ Mousy — Architecture, Roadmap & Next Steps Documentation

Welcome to the comprehensive roadmap and improvement guide for **Mousy**, the ultra-modern gaming mouse configuration, diagnostics, and training suite built with **C++17** and **Qt6**.

This document outlines:
1. **Current System Architecture & Features**
2. **Immediate Improvements & Low-Hanging Fruits**
3. **Advanced Features & Planned Modules**
4. **Hardware Driver & Linux System Integration (evdev / uinput / libratbag / hidapi)**
5. **Cross-Platform & Distribution Roadmap**
6. **Task Priority Matrix**

---

## 1. 🏗️ Current Architecture Overview

Mousy is structured into high-performance, modular Qt components:

```
mousy/
├── CMakeLists.txt                 # Auto-versioning, Git metadata extraction, Qt6 linking
├── VERSION                        # Semantic version anchor (SemVer 2.0.0)
├── .github/workflows/release.yml  # Automated GitHub Actions CI/CD release pipeline
├── scripts/release.sh             # Interactive local release bump & tag tool
└── src/
    ├── main.cpp                   # Global Qt Application & dark cyberpunk QSS styling
    ├── MainWindow.h/.cpp          # Tab management, status bar, and central coordination
    ├── Version.h.in               # CMake template generating dynamic build metadata
    ├── MouseViewWidget.h/.cpp     # Custom 3D perspective mouse model, picking, RGB engine
    ├── HeatmapWidget.h/.cpp       # Dual-layer motion & click density canvas with PNG export
    ├── AimTrainerWidget.h/.cpp    # 3 game modes, particle system, reaction time tracking
    └── MacroWidget.h/.cpp         # Macro recorder, loop multiplier, speed scaler, presets
```

---

## 2. ⚡ Immediate Improvements (Next Sprint)

### A. 3D Model & Visual Fidelity
1. **OBJ/GLTF 3D Model Loader**:
   - Currently, the 3D mouse is built using custom parametric/faceted 3D geometry in `MouseViewWidget`.
   - *Next Step*: Add a lightweight parser (or tinyobjloader) to allow users to load `.obj` or `.stl` meshes of actual gaming mice (e.g., Logitech G Pro, Razer DeathAdder, Glorious Model O).
2. **Inertia & Momentum Smoothing**:
   - Add momentum to rotation so when dragging and releasing quickly, the model continues to smoothly spin and decelerate.
3. **Custom Per-Key RGB Lighting**:
   - Allow setting individual RGB colors for each button, scroll wheel, and the logo zone.

### B. Button Remapping & Persistence
1. **Configuration Persistence (`settings.json`)**:
   - Use `QSettings` or `QJsonDocument` to save keybindings, chosen RGB effects, DPI, and camera angles across application restarts (`~/.config/mousy/config.json`).
2. **Profile Switcher**:
   - Add a profile bar at the top (`Default`, `FPS Gaming`, `Productivity`, `MMO`) allowing instant switching between button layouts.

### C. Heatmap Enhancements
1. **Global Background Tracking**:
   - Currently, the heatmap tracks cursor movement *within* the application window.
   - *Next Step*: Add an optional background service/daemon using `X11` (`XRecord` / `XQueryPointer`) or Wayland protocols to track mouse movements across the entire desktop.
2. **Resolution & Density Filters**:
   - Allow toggling gradient schemes (e.g., Thermal Rainbow, Neon Cyan-Magenta, Classic Fire).

### D. Aim Trainer Enhancements
1. **High Score Leaderboard**:
   - Persist top scores, personal records, and accuracy percentages per mode (*Classic*, *Flick*, *GridShot*).
2. **Custom Target Audio Effects**:
   - Play tactile click / hit marker sound effects on hit using `QSoundEffect` or `QAudioSink`.

---

## 3. 🚀 Advanced Features Roadmap

### 1. 🖲️ Hardware-Level Mouse Remapping (`evdev` & `uinput`)
Currently, key bindings are stored logically in the UI. To make remaps apply system-wide in any game or app:
- **Linux `evdev` Grabber**: Read raw input events from `/dev/input/event*`.
- **Linux `uinput` Virtual Device**: Re-emit translated events (e.g., convert Mouse Button 4 to `Ctrl + Shift + T`).
- **Permissions Helper**: Add a `pkexec` or `udev` rule script (`99-mousy.rules`) to grant access without needing `sudo`.

### 2. 🔌 Gaming Mouse Hardware Integration (libratbag & HIDAPI)
Support direct hardware memory flashing and on-board sensor adjustment:
- **`libratbag` (DBus integration)**:
  - Communicate with `ratbagd` to read and write directly to Logitech, SteelSeries, Roccat, and Razer on-board memory.
  - Read actual hardware DPI steps, sensor lift-off distance (LOD), and battery levels for wireless mice.
- **Direct USB HID (`hidapi`)**:
  - Send vendor-specific packets for direct RGB firmware control.

### 3. 📊 Sensor Diagnostics & Polling Rate Tester
Add a dedicated **Diagnostics** tab:
- **True Polling Rate Tester**: Measure real USB polling frequency (Hz) and jitter/stability curves.
- **Sensor Tracking Speed (IPS) & Acceleration Test**: Graph movement velocity in real-time to detect sensor spin-outs or hardware acceleration anomalies.
- **Click Latency Benchmark**: Measure mechanical switch debouncing and click response delay.

### 4. 🎛️ Audio Visualizer RGB Mode
- Hook into desktop audio (via PulseAudio / PipeWire) to pulse the mouse's RGB underglow in sync with music or game audio.

---

## 4. 📋 Step-by-Step Implementation Guide

| Order | Task | Complexity | Impact |
|:---:|:---|:---:|:---:|
| **1** | **Config Persistence (`QSettings` / JSON)**: Auto-save button bindings & settings | Low | ⭐⭐⭐⭐⭐ |
| **2** | **Multiple Profiles**: Allow profile creation (Gaming, Work, Editing) | Medium | ⭐⭐⭐⭐ |
| **3** | **Audio FX in Aim Trainer**: Add punchy hit/miss sound effects | Low | ⭐⭐⭐⭐ |
| **4** | **Polling Rate & Jitter Benchmark Tab**: Test real mouse sensor frequency | Medium | ⭐⭐⭐⭐⭐ |
| **5** | **Udev Rule & `uinput` Remapping**: Real system-wide button translation | High | ⭐⭐⭐⭐⭐ |
| **6** | **Custom 3D Mesh Loader**: Load custom mouse `.obj` models | High | ⭐⭐⭐⭐ |
| **7** | **libratbag / ratbagd Integration**: Flash onboard hardware memory | High | ⭐⭐⭐⭐⭐ |

---

## 5. 🛠️ Development & Contribution Workflow

### Building & Running
```bash
git clone https://github.com/MattiaFermani/mousy.git
cd mousy
cmake -B build -S .
cmake --build build
./build/Mousy
```

### Releasing New Versions
Mousy uses automated Semantic Versioning:
```bash
# Bug fixes & polish:
./scripts/release.sh patch "Description of fixes"

# New features & tabs:
./scripts/release.sh minor "Description of new feature"

# Major architectural updates:
./scripts/release.sh major "Description of major rewrite"
```

The script automatically bumps `VERSION`, re-generates `Version.h`, tags git, compiles release assets, and publishes to GitHub Releases!
