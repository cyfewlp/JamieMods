# Jamie Mods

A personal collection of mods for Skyrim Special Edition / Anniversary Edition.
The primary project is **SimpleIME** — adds IME support so Chinese, Japanese, Korean and other
multi-byte languages can be typed in-game.

[![Nexus Mods](https://img.shields.io/badge/NexusMods-SimpleIME-orange?style=flat-square&logo=nexusmods)](https://www.nexusmods.com/skyrimspecialedition/mods/140136)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg?style=flat-square)](https://opensource.org/licenses/MIT)

## Projects

| Project                          | Description                               |
|----------------------------------|-------------------------------------------|
| [SimpleIME](SimpleIME/README.md) | Native IME input support for Skyrim SE/AE |

## Getting started

```bash
git clone --recursive https://github.com/cyfewlp/JamieMods.git
cd JamieMods
```

If you already cloned without `--recursive`:

```bash
git submodule update --init --recursive
```

## Requirements

- [Visual Studio 2022](https://visualstudio.microsoft.com/) (Community edition is fine)
- [LLVM](https://github.com/llvm/llvm-project) — provides `clang-cl`, `clang-format`, `clang-tidy`
- [CMake](https://cmake.org/) ≥ 4.2
- [vcpkg](https://github.com/microsoft/vcpkg)
  - Clone or download, run `bootstrap-vcpkg.bat`, then set `VCPKG_ROOT` to the vcpkg folder.

## Build

See [SimpleIME/README.md](SimpleIME/README.md) for project-specific configure, build, and test instructions.

## Python tools

Some scripts under `scripts/` and `SimpleIME/` require Python. Set up the environment once:

```shell
python -m venv .venv
.venv\Scripts\Activate.ps1   # PowerShell
pip install -r requirements.txt
```

Notable scripts:

- `SimpleIME/extract_i18n.py` — checks translation key coverage; warns on missing or unused keys.

## Credits

| Library | Purpose |
|---------|---------|
| [CommonLibSSE-NG](https://github.com/alandtse/CommonLibVR) | Skyrim reverse-engineering backbone |
| [Dear ImGui](https://github.com/ocornut/imgui) | In-game UI rendering |
| [material-color-utilities](https://github.com/material-foundation/material-color-utilities) | Material You dynamic theming |
| [ImThemes](https://github.com/Patitotective/ImThemes) | ImGui theme browser / exporter |
| [Lucide](https://github.com/lucide-icons/lucide) | Icon SVGs (repackaged into a minimal TTF) |
