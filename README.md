# 🖱 Mousy — Advanced Mouse Editor

A sleek, ultra-modern mouse configuration and training tool built with **C++ / Qt6**.

![C++](https://img.shields.io/badge/C++-17-blue?logo=cplusplus)
![Qt](https://img.shields.io/badge/Qt-6-green?logo=qt)
![License](https://img.shields.io/badge/License-MIT-yellow)

## Features

### 🖱 Interactive Button Mapper
- Visual top-down mouse diagram with clickable button regions
- Neon hover effects and depth rendering
- Remap any mouse button to a custom key or shortcut
- Reset individual buttons to defaults

### 🔥 Live Heatmap
- Real-time mouse movement tracking with multi-color heat gradient
- Click density visualization with Gaussian splatting
- Smooth heat decay over time
- Clear and reset functionality

### 🎯 Aim Trainer
- Pulsing neon targets with bullseye rings and life bars
- Configurable target size and spawn speed
- Live stats: hits, misses, avg reaction time, accuracy %
- Color-coded accuracy feedback

### ⏺ Macro Recorder
- Record mouse clicks and keyboard events with precise timing
- Play back macros with original delays
- Save and load macros to `.mousy` files
- Edit recorded events individually

## Building

### Prerequisites

- CMake 3.16+
- Qt 6 (Core, Gui, Widgets)
- A C++17 compiler (GCC, Clang, MSVC)

### Ubuntu / Debian

```bash
sudo apt install cmake build-essential qt6-base-dev
```

### Build & Run

```bash
cmake -B build -S .
cmake --build build
./build/Mousy
```

## License

MIT
