# Jamie Mods

A personal collection of small mods and development examples for Skyrim.
The primary project in this repository is **SimpleIME**, which adds IME (Input Method Editor)
support to **Skyrim Special Edition** (SE) / **Anniversary Edition** (AE) so languages such as
Chinese and Japanese can be entered in-game.

[![Nexus Mods](https://img.shields.io/badge/NexusMods-SimpleIME-orange?style=flat-square&logo=nexusmods)](https://www.nexusmods.com/skyrimspecialedition/mods/140136)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg?style=flat-square)](https://opensource.org/licenses/MIT)

## 📦 Project Overview

### 🌟 [SimpleIME](SimpleIME/README.md)

The primary project in this repository. **SimpleIME** provides native IME (Input Method Editor) support for Skyrim
SE/AE, enabling players to type in Chinese, Japanese, Korean, and other multi-byte languages within the game console and
UI.

## Quick start

Clone recursively to ensure any submodules are retrieved:

```bash
git clone --recursive https://github.com/cyfewlp/JamieMods.git
cd JamieMods
```

If you already cloned without `--recursive`, fetch submodules with:

```bash
git submodule update --init --recursive
```

## Build requirements

- [Visual Studio 2022](https://visualstudio.microsoft.com/) (_the free Community edition_)
- [LLVM](https://github.com/llvm/llvm-project)
    - clang-format
    - clang-tidy
    - clang-cl(the compiler)
- [CMake](https://cmake.org/)
- [`vcpkg`](https://github.com/microsoft/vcpkg)
    1. Clone the repository using git
       OR [download it as a .zip](https://github.com/microsoft/vcpkg/archive/refs/heads/master.zip)
    2. Go into the `vcpkg` folder and double-click on `bootstrap-vcpkg.bat`
    3. Edit your system or user Environment Variables and add a new one:
        - Name: `VCPKG_ROOT`    
          Value: `C:\path\to\wherever\your\vcpkg\folder\is`

<img src="https://raw.githubusercontent.com/SkyrimDev/Images/main/images/screenshots/Setting%20Environment%20Variables/VCPKG_ROOT.png" height="150" alt="">

## Build & open in IDE

Please check Project-specific README.md

## 🛠️ Debug Setup: Configuring MO2 Path

This project includes a pre-configured CLion run task (Run MO2 SKSE) that launches the game through `Mod Organizer 2`.
To
use this task, you must define the `MO2Path` variable in your CLION IDE settings to point to your local MO2 installation
folder.

How to set the path variable:

- Open Settings (or Preferences on macOS) by pressing **Ctrl+Alt+S**.
- Navigate to **Appearance & Behavior > Path Variables**.
- Click the **+** (Add) button.
- In the Name field, enter exactly: **MO2Path**
- In the Value field, click the folder icon and select the root directory where your `ModOrganizer.exe` is located (
  e.g., `C:
/Games/Mod Organizer 2`).
- Click OK and Apply.

## 🎨 Dynamic Theming with Material You

`SimpleIME` features a high-performance, dynamic theming engine powered by Google's [material-color-utilities](https://www.google.com/search?q=https://github.com/google/material-color-utilities). Instead of relying on static color presets, the project generates comprehensive, perceptually accurate UI themes in real-time based on a single "Seed Color."

This looks like a solid integration of Material 3 design principles into a C++ environment. Using a popup picker to live-calculate an entire theme from a single seed color is exactly how Material You is intended to function.

Here is a professionally drafted **README** section in English. It explains the integration of `material-color-utilities`, emphasizes the dynamic nature of your `SimpleIME` project, and includes relevant code snippets for clarity.

---

## 🎨 Dynamic Theming with Material You

`SimpleIME` features a high-performance, dynamic theming engine powered by Google's [material-color-utilities](https://www.google.com/search?q=https://github.com/google/material-color-utilities). Instead of relying on static color presets, the project generates comprehensive, perceptually accurate UI themes in real-time based on a single "Seed Color."

### Key Features

- **Perceptual Color Logic**: Utilizes the **HCT (Hue, Chroma, Tone)** color space to ensure consistent contrast and accessibility across all generated themes.
- **Live UI Customization**: Users can select any color via a picker, and the entire interface—including Primary, Surface, and Container roles—recalculates instantly to match.
- **Dark & Light Mode Support**: A single click toggles between themes while maintaining the semantic relationship between color roles.
### Technical Implementation

The project implements a custom `ThemeBuilder` that bridges the Material Color Utilities (MCU) with the ImGui rendering pipeline.
1.  **Generating Themes from Seed**
   The core logic utilizes `SchemeTonalSpot` to generate balanced palettes. We perform manual bit-shifting to convert MCU's `ARGB` format into ImGui's expected color layout:

```cpp
void ThemeBuilder::BuildThemeFromSeed(uint32_t seedArgb, bool isDark, Colors &colors) {
    using namespace material_color_utilities;
    
    // Generate a Tonal Spot scheme based on the seed color and brightness mode
    SchemeTonalSpot scheme(Hct(seedArgb), isDark, 0.0);

    // Helper to bridge Material ARGB (0xAARRGGBB) to ImGui (0xAABBGGRR / ImColor)
    auto argbToImColor = [](const Argb argb) -> ImU32 {
        uint8_t a = (argb >> 24) & 0xFF;
        uint8_t r = (argb >> 16) & 0xFF;
        uint8_t g = (argb >> 8)  & 0xFF;
        uint8_t b = argb & 0xFF;
        return ImColor(r, g, b, a);
    };

    colors.primary          = argbToImColor(scheme.GetPrimary());
    colors.onPrimary        = argbToImColor(scheme.GetOnPrimary());
    colors.surfaceContainer = argbToImColor(scheme.GetSurfaceContainer());
    // ... further mappings
}
```

2. **Interactive Theme Editor**
Within the **Appearance Panel**, users can interactively preview and apply new color schemes. The implementation ensures that color data flows seamlessly between the standard `ImGui::ColorPicker4` (floats) and the MCU engine (uint32).

```cpp
// Within the UI loop
if (ImGui::Button("Apply Theme")) {
    // Convert ImU32 back to MCU-compatible ARGB before processing
    uint32_t seed = imU32ToArgb(m_colorInThemeBuilder);
    ThemeBuilder::BuildThemeFromSeed(seed, darkMode, m_styles.colors);
}
```

### Why Material 3?

By leveraging Google's research into color science, `SimpleIME` avoids the common pitfall of "clashing colors" in custom themes. Whether you choose a vibrant neon or a muted pastel, the algorithm ensures that text remains readable and the UI hierarchy remains clear.
## 📝 Documentation & Contribution Guide

This repository is optimized for [Obsidian](https://obsidian.md).
We use it to track bugs, features, and technical notes.

### How we use Obsidian:

Tagging System: We use nested tags (e.g., `#status/todo`, `#type/bug`) to manage the development lifecycle.

* Backlinks: Technical modules are connected via [[Double Brackets]] for better navigation.
* Task Tracking: Please refer to our Tagging Convention Guide before contributing or logging issues.

Please check [[TAGGING_GUIDE]]

## 🤝 Credits & Libraries

* [CommonLibSSEVR/ng](https://github.com/alandtse/CommonLibVR): The backbone library for Skyrim reverse engineering.
* [Dear ImGui](https://github.com/ocornut/imgui): Used for in-game overlays.
* [ImThemes](https://github.com/Patitotective/ImThemes): ImGui styling browser.
* [material-color-utilities](https://github.com/material-foundation/material-color-utilities.git): Color libraries for Material You
* [SimpleIni](https://github.com/brofield/simpleini): Configuration file parser.
* [Nerd Font](https://github.com/ryanoasis/nerd-fonts): Iconic font aggregator.