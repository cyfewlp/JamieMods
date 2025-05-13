# ImGui for SkyrimOutfitSystemSE (SosGui)

Main Window:

![[main_window.png]]

Outfit Edit Panel

![[outfit_edit_panel.png]]
## Features

* Armor candidates
    * support multi-selection;
    * support sort armor by name with Unicode string;
    * highlight conflict armor with current outfit armors;

* UiSettings
    * Store in `imgui.ini` with custom ini handlers
    * Support dynamic scale global font size by `imgui fetature/dynamic-fonts` branch;


## Requirements

* CMAKE
* [Skyrim Outfit System SE Revived](https://www.nexusmods.com/skyrimspecialedition/mods/42162)
  > A mod that allows you to change the visual appearance of your armor while keeping the stats of your real armor.
  > An updated version of DavidJCobb and aers's work.
* [Dear ImGui](https://github.com/ocornut/imgui)
  > Dear ImGui: Bloat-free Graphical User interface for C++ with minimal dependencies
* [ImThemes](https://github.com/Patitotective/ImThemes)
  > Dear ImGui style browser and editor written in Nim

  This mod will load `ImThemes/themes.toml` and parse all available themes.
## Environment Varibles

`MO2_MODS_PATH`: The `ModOrganizer2` mods folder. The `dll`, `pdb` and other required filed will auto copy to `MO2_MODS_PATH/{PLUGIN_NAME}` folder when build successful if seup this env varible;
## Build

```shell
```shell
cmake --preset debug-clangcl-ninjia-vcpkg
cmake --build build\debug-clangcl-ninjia-vcpkg --target SosGui
cd build\debug-clangcl-ninjia-vcpkg
cpack
```
## Document

Use [Obsidian](https://obsidian.md/) as document manage tool: `Wikilink` and `Backlink`