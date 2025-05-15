## v0.0.1

## ChangeLogs

* [x] [[Bug Fix]] when click `Import Setting`, may have reference released outfit;
* [x] [[Bug Fix]]: click mod name filter: throw exception: `invalid vector subscript`
* [ ] [[New Feature]]: Support edit empty outfit?
* [ ] [[improvement]] Refactor error message system.
* [x] [[Bug Fix]]: The `viewData` is not cleared correctly when click `Import Settings` for the second time.
* [x] Support adds armor from near all objects has inventory;
* [ ] [[improvement]] Font: monaspace
* [ ] [[improvement]] how to handle current actor active outfit?
* [x] [[improvement]] Use `consteval` optimize `ImGuiUtil`  flags wrap class
* [x] [[Bug Fix]] `ImGui` assert fail when scroll [[OutfitListView]].
* [x] [[Bug Fix]] Can't found armor in `ArmorContainer`  after `Import Settings`
	* `ArmorContainer` 
* [x] [[improvement]] setup our `imgui.ini`: `Data\interface\SosGui\imgui.ini`;
* [x] [[improvement]] Add `Context` class: hold `popupList` field: All GUI share this `popupList`.
* [x] [[improvement]] Disable interactive with armor view when outfit is `Untitled`;
* [x] [[improvement]] Add [NerdFont](https://github.com/ryanoasis/nerd-fonts) and use the `SymbolsNerdFontMono-Regular` font as icon font;
	* add `stb_image.h` to #CommonModule
	* add `iconFont` field to `Context` class: setup in initialize `ImGui`;
* [x] [[Bug Fix]] Other Actor not setup active outfit when reopen GUI;
* [x] [[Bug Fix]] Missing call `EndChild`: trigegr when click delete actor
* [x] [[Bug Fix]] `OutfitService#GetActorAllStateOutfit`;
* [x] [[improvement]] Refactor `m_autoSwitchOutfitSelectPopup`, `m_outfitSelectPopup`: Use `Context#popupList`;
* [x] Test `AutoSwitchPolicyView` emplace;
	* Add function `emplace_or_replace` to support replace existed key-value;
* [x] [[improvement]] Optimize wait_execute_on_ui?
	* `coroutine` resume always on `MainThread` and `IMenu#PostDisplay` also called on `MainThread`. So, remove all `await_execute_on_ui` calls ;
* [ ] [[improvement]] Optimize data class: ensure **DONOT** modify `DATA` class on UI thread;
	* ~~consider use read/write lock~~
* [x] [[improvement]] `UiSetting`: be used hold the persist settings. May merge `Config` class to `Setting`
	* Add `UiSettings#DefaultThemeIndex` to support select `ImGui` provide default theme;
	* Call `ImGui#MarkIniSettingsDirty` when select a theme;
* [x] [[improvement]] Sort Themes; Support select `ImGui` Default theme: classic, dark, light
- [x] [[improvement]] cleanup `ImGui` when game quit
	- ✅hook `WndProc` and process `WM_DESTROY` message?
	- ❔Is game provided quit event?
- [x] [[Bug Fix]] Game can't quit: Is Introduced by `ImGui#Shutdown` by `WM_DESTORY` ?
- [x] [[improvement]] search icon for all search input widgets;
- [x] [[improvement]] Add filters icon:
	- Click will open a popup: show all filters checkbox
- [ ] [[improvement]] Bold font: bold font for table header;
- [x] [[improvement]] Move font scale, Theme config to Settings menu item;
- [x] [[Bug Fix]] Theme was not applied correctly if theme index was set to `DefaultThemeIndex`;
- [x] [[Bug Fix]] Mod refrence counter incorrect;
- [ ] [[Bug Fix]] `SosNativeCaller` may resume on other thread;
- [ ] [[improvement]] when Click `Sidebar`: Focus the associate window if it's not focused 
- [ ] [[improvement]] About popup
- [ ] [[improvement]] shadow border for popup