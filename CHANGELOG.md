## [0.1.0] - 2026-03-30

### 🚀 Features

- *(imguiex)* Only support imgui v1.92.6
- *(cmake,SosGui)* Add add_build_lucide_icons cmake function to repacke lucide icons from a icon list file;
- Merge sosgui

### 🐛 Bug Fixes

- *(imguiex/M3)* Default ui scale is 1.0F now

### 💼 Other

- Remove all simpleime build config;
- Fix the imgui-config.cmake
- By Papyrus event call SkyrimOutfitSystemSE functions
- Use TabBar replace RadioButton;
- Fix: move the code that restores outfit armor to view to after the batch add code
- ToolBar add a Views menu to manual control view showing;
- Refactor Popups: popup now only hold on demand;
- Add wrap class RankedArmor to speed up search the armor rank;
- Add ImGuiScop;
- Format
- Format
- Use `consteval` optimize `ImGuiUtil`  flags wrap class
- * Move imgui flags wrap class to common module
- * Disable interactive with armor view when outfit is `Untitled`;
- * Merge Config to UiSetting
- * remove EditPanel PushId*this): cause imgui.ini continuously expanding
- * use icon NF_OCT_DIFF_ADDED
- * update UI layout
- AboutPopup
- * directly compile ImGui
- Add UiSettingsLoader
- Optimize FontManager & UiSettings
- Focus window if window not focused when click the window icon button in the sidebar
- Move `Enable` & `Quickslot` to File menu;
- Directly erase outfit when unmark favorite outfit
- Add new gui CharacterEditPanel to replace SosGuiCharacters &SosGui;
- Refresh icon for RefreshPlayerArmor
- Update assets

### 🚜 Refactor

- Fix all build errors
- *(SimpleIME)* Move FontManager to common module; move header-only lib to extern directory;
- *(SimpleIME)* Move the imgui_system to common lib imgui_manager
- *(SosGui)* Remove font manager module; Migrate to common FontManager; Migrated to imgui_manager
- *(SosGui)* Remove About/SettingsPopup; Migrated to lucide icons;
- *(SimpleIME)* Migrate translator_manager to the common module;
- *(SosGui)* Depecated translator module and Migrate to i18n::Translator(WIP)

### ⚙️ Miscellaneous Tasks

- Remove SimpleIME, now lives in its own repo
- Update README
- Remove sosgui,simpleIME
