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

## 📜 License

MIT License
