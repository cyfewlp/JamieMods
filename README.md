# Jamie Mods

This repository is my mods collection;

- [SimpleIME](SimpleIME/README.md): Make SkyrimSE/AE support IME.
## Clone

```shell  
git clone --recursive https://github.com/cyfewlp/JamieMods```  
  
- [CommonLibSSE NG](#commonlibsse-ng)  
- [Requirements](#requirements)  
  - [Opening the project  
  
# CommonLibSSE NG  
  
Because this uses [CommonLibSSE NG](https://github.com/CharmedBaryon/CommonLibSSE-NG), it supports Skyrim SE, AE, GOG, and VR.  
  
[CommonLibSSE NG](https://github.com/CharmedBaryon/CommonLibSSE-NG) is a fork of the popular [powerof3 fork](https://github.com/powerof3/CommonLibSSE) of the _original_ `CommonLibSSE` library created by [Ryan McKenzie](https://github.com/Ryan-rsm-McKenzie) in [2018](https://github.com/Ryan-rsm-McKenzie/CommonLibSSE/commit/224773c424bdb8e36c761810cdff0fcfefda5f4a).  
  
# Requirements  
  
- [Visual Studio 2022](https://visualstudio.microsoft.com/) (_the free Community edition_)  
- [LLVM](https://github.com/llvm/llvm-project)  
  - clang-format  
  - clang-tidy  
  - clang-cl(the compiler)  
- [CMake](https://cmake.org/)  
- [`vcpkg`](https://github.com/microsoft/vcpkg)  
  - 1. Clone the repository using git OR [download it as a .zip](https://github.com/microsoft/vcpkg/archive/refs/heads/master.zip)  
  - 2. Go into the `vcpkg` folder and double-click on `bootstrap-vcpkg.bat`  
  - 3. Edit your system or user Environment Variables and add a new one:  
    - Name: `VCPKG_ROOT`    
Value: `C:\path\to\wherever\your\vcpkg\folder\is`  
  
<img src="https://raw.githubusercontent.com/SkyrimDev/Images/main/images/screenshots/Setting%20Environment%20Variables/VCPKG_ROOT.png" height="150">  
## Credits  
  
[Nerd Font]([ryanoasis/nerd-fonts: Iconic font aggregator, collection, & patcher. 3,600+ icons, 50+ patched fonts: Hack, Source Code Pro, more. Glyph collections: Font Awesome, Material Design Icons, Octicons, & more](https://github.com/ryanoasis/nerd-fonts)) (MIT License (MIT)): The amazing icon font! [[SosGui]] Use the `Symbols Nerd Font`;
[ImThemes](https://github.com/Patitotective/ImThemes): Dear ImGui style browser and editor written in Nim
[toml++](https://github.com/marzer/tomlplusplus): Header-only TOML config file parser and serializer for C++17.