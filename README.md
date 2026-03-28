# JamieMods

Shared CMake library used by my Skyrim SE/AE mod projects.
Provides common build infrastructure, ImGui extensions (MD3 component library),
Material You theming, and utilities shared across mods.

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg?style=flat-square)](https://opensource.org/licenses/MIT)

## Projects using this library

| Project                                           | Description                               |
| ------------------------------------------------- | ----------------------------------------- |
| [SimpleIME](https://github.com/cyfewlp/SimpleIME) | Native IME input support for Skyrim SE/AE |

## Contents

| Directory  | Description                                                                   |
| ---------- | ----------------------------------------------------------------------------- |
| `cmake/`   | Shared CMake modules (`imgui-config.cmake`, `version.rc.in`, `material-you/`) |
| `imguiex/` | ImGui extension library — Material Design 3 component set                     |
| `common/`  | Shared C++ headers (logging, hooks, path utils, i18n)                         |
| `extern/`  | Third-party submodules (`material-color-utilities`, `imgui_test_engine`)      |
| `scripts/` | Build-time Python scripts (icon font generation, etc.)                        |

## Usage as a submodule

```bash
git submodule add https://github.com/cyfewlp/JamieMods extern/JamieMods
git submodule update --init --recursive
```

In your `CMakeLists.txt`:

```cmake
add_subdirectory(extern/JamieMods EXCLUDE_FROM_ALL)
include(extern/JamieMods/cmake/imgui-config.cmake)
```

## Requirements

- [Visual Studio 2022](https://visualstudio.microsoft.com/) (Community edition is fine)
- [LLVM](https://github.com/llvm/llvm-project) — provides `clang-cl`, `clang-format`, `clang-tidy`
- [CMake](https://cmake.org/) ≥ 4.2
- [vcpkg](https://github.com/microsoft/vcpkg)
  - Clone or download, run `bootstrap-vcpkg.bat`, then set `VCPKG_ROOT` to the vcpkg folder.

## Python tools

Some build scripts require Python. Set up the environment once:

```shell
python -m venv .venv
.venv\Scripts\Activate.ps1   # PowerShell
pip install -r requirements.txt
```

## Third-party dependencies

| Library                                                                                     | License    | Purpose                        |
| ------------------------------------------------------------------------------------------- | ---------- | ------------------------------ |
| [material-color-utilities](https://github.com/material-foundation/material-color-utilities) | Apache-2.0 | Material You dynamic theming   |
| [imgui](https://github.com/ocornut/imgui)                                                   | MIT        | Immediate-mode GUI             |
| [ImThemes](https://github.com/Patitotective/ImThemes)                                       | MIT        | ImGui theme browser / exporter |
