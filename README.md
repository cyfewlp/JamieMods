# Jamie Mods

A personal collection of small mods and development examples for Skyrim.
The primary project in this repository is **SimpleIME**, which adds IME (Input Method Editor)
support to **Skyrim Special Edition** (SE) / **Anniversary Edition** (AE) so languages such as
Chinese and Japanese can be entered in-game.

[![Nexus Mods](https://img.shields.io/badge/NexusMods-SimpleIME-orange?style=flat-square&logo=nexusmods)](https://www.nexusmods.com/skyrimspecialedition/mods/140136)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg?style=flat-square)](https://opensource.org/licenses/MIT)

## 📦 Project Overview

### 🌟 [SimpleIME](SimpleIME/README.md)
The primary project in this repository. **SimpleIME** provides native IME (Input Method Editor) support for Skyrim SE/AE, enabling players to type in Chinese, Japanese, Korean, and other multi-byte languages within the game console and UI.

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

## 📝 Documentation & Contribution Guide

This repository is optimized for Obsidian. We use it to track bugs, features, and technical notes.

### How we use Obsidian:


Tagging System: We use nested tags (e.g., `#status/todo`, `#type/bug`) to manage the development lifecycle. 
* Backlinks: Technical modules are connected via [[Double Brackets]] for better navigation.
* Task Tracking: Please refer to our Tagging Convention Guide before contributing or logging issues.

Please check [[TAGGING_GUIDE]]
## 🤝 Credits & Libraries
* [CommonLibSSEVR/ng](https://github.com/alandtse/CommonLibVR): The backbone library for Skyrim reverse engineering.
* [Dear ImGui](https://github.com/ocornut/imgui): Used for in-game overlays.
* [ImThemes](https://github.com/Patitotective/ImThemes): ImGui styling browser.
* [SimpleIni](https://github.com/brofield/simpleini): Configuration file parser.
* [Nerd Font](https://github.com/ryanoasis/nerd-fonts): Iconic font aggregator.