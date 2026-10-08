# 🖱 Mousy — Advanced Mouse Editor

A sleek, ultra-modern gaming mouse configuration, diagnostics, and training tool built with **C++17 / Qt6**.

![C++](https://img.shields.io/badge/C++-17-blue?logo=cplusplus)
![Qt](https://img.shields.io/badge/Qt-6-green?logo=qt)
![License](https://img.shields.io/badge/License-MIT-yellow)

---

## ⚡ Features

### 🖱 Interactive Button Mapper & Lighting Studio
- **3D Top-Down Mouse Visualizer**: Rendered with dynamic shadows, depth gradients, and smooth 60fps pulsing effects.
- **Clickable Hotspots**: Map buttons directly on the diagram (Left, Right, Wheel, Forward, Back, and DPI).
- **Custom Key & Shortcut Remapping**: Instantly remap mouse buttons to keyboard keys or shortcuts with live visual overlay on the mouse.
- **RGB Lighting Effects**: Switch between 5 dynamic lighting modes (*Neon Cyan*, *Rainbow Cycle*, *Cyberpunk Pink*, *Matrix Green*, *Crimson Fire*) with real-time underglow and animated LED light strips.
- **Hardware Profiles**: Configure DPI stages (400 to 6400 DPI) and Polling Rates (125Hz, 500Hz, 1000Hz).

### 🔥 Real-Time Heatmap & Motion Tracker
- **Multi-layer Heat Density**: Dual heat tracking separating smooth motion trails and high-impact click clusters using Gaussian splatting.
- **Fading Cursor Trails**: Real-time neon cyan path visualization tracing your cursor's exact movements.
- **View Filters**: Toggle between *Combined*, *Movement Only*, and *Clicks Only*.
- **Snapshot Export**: Export your heatmap directly to high-resolution PNG snapshots with a single click.
- **Live Counter**: Track total moves and clicks in real time.

### 🎯 Pro Aim Trainer
- **Game Modes**:
  - **Classic**: Random spatial target spawning.
  - **Flick**: Rapid edge-spawning drills designed to train flick shots.
  - **GridShot**: Tactical 3x3 grid precision drills.
- **Juicy Visual FX**: Pulsing neon targets, target shrink/life bars, dynamic particle burst explosions, and floating reaction feedback (*INSANE!*, *GREAT!*, *GOOD*).
- **Streak & Combo Multiplier**: Track current kill streaks with animated fire effects and all-time session bests.
- **Detailed Analytics**: Real-time tracking of Hits, Misses, Average Reaction Time (ms), and Accuracy (%).

### ⏺ Macro Studio
- **Precision Event Recording**: Capture key presses, key releases, and mouse coordinates with microsecond-level timing.
- **Quick Presets**: Instant generators for *Double Click*, *Rapid Fire (5x)*, *Custom Text Sequences*, and *Delay Injectors*.
- **Playback Controls**: Adjust loop repetitions (1x to 100x) and speed multipliers (0.5x slow motion up to 4.0x hyperspeed).
- **File Persistence**: Save and reload custom macro chains in `.mousy` format.

---

## 🛠 Building & Installation

### Prerequisites

- CMake 3.16+
- Qt 6 (`qt6-base-dev`)
- C++17 compatible compiler (GCC, Clang, MSVC)

### Ubuntu / Debian

```bash
sudo apt update
sudo apt install cmake build-essential qt6-base-dev
```

### Build & Run

```bash
git clone https://github.com/MattiaFermani/mousy.git
cd mousy
cmake -B build -S .
cmake --build build
./build/Mousy
```

---

## 🏷 Versioning & Release Policy

Mousy follows the **[Semantic Versioning 2.0.0 (SemVer)](https://semver.org/)** specification: `vMAJOR.MINOR.PATCH`.

| Component | Nomenclature | Meaning & Trigger |
| :--- | :--- | :--- |
| **MAJOR** | `vX.0.0` | **Architectural Shifts**: Breaking changes, fundamental core rewrites, or major UX restructuring. |
| **MINOR** | `v1.X.0` | **Subversion / Features**: New feature sets, diagnostic tabs, hardware integrations, or lighting engines. |
| **PATCH** | `v1.0.X` | **Revision / Fixes**: Backwards-compatible bug fixes, UI styling polish, and calibration tweaks. |
| **BUILD** | `+commit` | Git commit SHA, branch metadata, and compilation timestamp embedded automatically by CMake. |

### 🚀 Cutting a Release

Releases are completely automated via GitHub Actions (`.github/workflows/release.yml`) or via the included release script:

```bash
# Bump patch (e.g. v1.0.0 -> v1.0.1)
./scripts/release.sh patch "Fix button glow offset"

# Bump minor / subversion (e.g. v1.0.0 -> v1.1.0)
./scripts/release.sh minor "Add Macro Studio loop controls"

# Bump major (e.g. v1.0.0 -> v2.0.0)
./scripts/release.sh major "New cross-platform driver architecture"
```

The script updates `VERSION`, re-generates `Version.h`, creates an annotated Git tag, pushes to GitHub, compiles release binaries, and publishes a new [GitHub Release](https://github.com/MattiaFermani/mousy/releases) with asset packages and checksums.

---

## 📜 License

MIT License
