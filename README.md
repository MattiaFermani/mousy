# 🖱 Mousy — Advanced Gaming Mouse Suite & Diagnostics

A sleek, ultra-modern gaming mouse configuration, diagnostics, and aim training suite built with **C++17 / Qt6**.

![C++](https://img.shields.io/badge/C++-17-blue?logo=cplusplus)
![Qt](https://img.shields.io/badge/Qt-6-green?logo=qt)
![License](https://img.shields.io/badge/License-MIT-yellow)

---

## ⚡ Features & Modules

### 🖱 1. Interactive 3D Viewport & Button Mapper
- **3D Real-Time Mouse Viewport**: Free 360° orbit rotation with momentum physics/inertia, smooth deceleration, zoom scaling (50%–250%), and preset camera angles (Isometric, Top, Flanks, Front).
- **Auto-Rotate Orbit Mode**: Cinematic showcase rotation for desktop setups.
- **Wavefront OBJ & STL Mesh Loader/Exporter**: Import your actual mouse 3D mesh (Logitech, Razer, Glorious, Zowie) or export the internal model to `.obj`.
- **Face & Button Picking**: Click directly on individual 3D buttons (LMB, RMB, Scroll Wheel, Forward, Back, DPI, Logo) to configure actions.
- **Per-Zone Custom RGB**: Select custom colors for individual chassis zones (Body, LMB, RMB, Wheel, Underglow) or choose from animated presets (*Neon Cyan*, *Rainbow Cycle*, *Cyberpunk Pink*, *Matrix Green*, *Crimson Fire*).

### 📊 2. Sensor Diagnostics & Latency Benchmark (Tab 5)
- **Live USB Polling Rate Monitor**: Real-time polling frequency measurement (125Hz, 500Hz, 1000Hz, up to 8000Hz) with live scrolling frequency graphs.
- **Polling Jitter & Stability Curve**: Microsecond interval jitter tracking to detect unstable USB ports or CPU throttling.
- **Tracking Velocity & IPS Speedometer**: Real-time sensor velocity calculation in Inches Per Second (IPS) and pixels/sec to test sensor tracking limits and prevent spin-outs.
- **Mechanical Switch Debounce & Latency Test**: Switch press/release duration and rapid double-click debounce health analyzer.
- **Linux Hardware Device Scanner**: Live `/sys/class/input` scanner identifying connected mouse names, USB vendor/product IDs, and bus paths.

### 🖲 3. Linux System Integration (`uinput` Virtual Device)
- **Global System-Wide Remapping**: Remapped buttons and macro combos are emitted directly to the Linux kernel via `/dev/uinput`, working globally across all games (Steam, Proton, Wine) and desktop apps.
- **Unprivileged Desktop Support**: Included `scripts/99-mousy.rules` udev rule grants user access without running the app as `root`.
- **Keyboard & Macro Emulation**: Translate mouse clicks into complex keyboard combos (e.g. `Ctrl+Shift+T`, `Alt+F4`, `F5`).

### 💾 4. Multi-Profile Persistence
- **Instant Profile Switcher**: Switch on-the-fly between layouts (`Default`, `FPS Gaming`, `Productivity`, or custom user profiles).
- **Auto-Persistence**: All DPI stages, polling rates, RGB lighting, 3D zone colors, and button bindings are automatically saved to `~/.config/mousy/profiles.json`.

### 🎯 5. Pro Aim Trainer with Procedural Audio FX
- **3 Dynamic Game Modes**: *Classic* (spatial reaction), *Flick* (edge flicking drills), and *GridShot* (tactical 3x3 speed drills).
- **Procedural 16-Bit PCM Audio Synthesizer**: Low-latency synthesized sound effects for countdown beeps, target hits, misses, and streak combo chimes via `QAudioSink`.
- **Streak & Combo Multiplier**: Dynamic fire particle effects and combo bonuses.
- **Personal Records Tracking**: Persistent personal best accuracy, top hits, and fastest reaction time stored per profile.

### 🔥 6. Real-Time Heatmap & Motion Tracker
- **Multi-layer Heat Density**: Dual heat tracking separating smooth motion trails and high-impact click clusters using Gaussian splatting.
- **Fading Cursor Trails**: Real-time neon cyan path visualization tracing your cursor's exact movements.
- **View Filters**: Toggle between *Combined*, *Movement Only*, and *Clicks Only*.
- **Snapshot Export**: Export your heatmap directly to high-resolution PNG snapshots with a single click.

### ⏺ 7. Macro Studio
- **Precision Event Recording**: Capture key presses, key releases, and mouse coordinates with microsecond-level timing.
- **Quick Presets**: Instant generators for *Double Click*, *Rapid Fire (5x)*, *Custom Text Sequences*, and *Delay Injectors*.
- **Playback Controls**: Adjust loop repetitions (1x to 100x) and speed multipliers (0.5x slow motion up to 4.0x hyperspeed).

---

## 🛠 Building & Installation

### Prerequisites

- CMake 3.16+
- Qt 6 (`qt6-base-dev`, `qt6-multimedia-dev`)
- C++17 compatible compiler (GCC, Clang)

### Ubuntu / Debian Installation

```bash
sudo apt update
sudo apt install -y cmake build-essential qt6-base-dev qt6-multimedia-dev libgl1-mesa-dev
```

### Build & Run

```bash
git clone https://github.com/MattiaFermani/mousy.git
cd mousy
cmake -B build -S .
cmake --build build
./build/Mousy
```

### Enabling Linux Hardware Remapping (`uinput`)

To allow Mousy to send virtual keystrokes and clicks without needing `sudo`:

```bash
sudo cp scripts/99-mousy.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules && sudo udevadm trigger
sudo modprobe uinput
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
# Bump patch (e.g. v1.1.0 -> v1.1.1)
./scripts/release.sh patch "Fix button glow offset"

# Bump minor / subversion (e.g. v1.1.0 -> v1.2.0)
./scripts/release.sh minor "Add libratbag hardware flashing support"

# Bump major (e.g. v1.1.0 -> v2.0.0)
./scripts/release.sh major "New cross-platform driver architecture"
```

The script updates `VERSION`, re-generates `Version.h`, creates an annotated Git tag, pushes to GitHub, compiles release binaries, and publishes a new [GitHub Release](https://github.com/MattiaFermani/mousy/releases) with asset packages and checksums.

---

## 📜 License

MIT License
