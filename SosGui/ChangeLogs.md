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
* [ ] [[Bug Fix]] `ImGui` assert fail when scroll [[OutfitListView]].
* [x] [[Bug Fix]] Can't found armor in `ArmorContainer`  after `Import Settings`
	* `ArmorContainer` 
* [x] [[improvement]] setup our `imgui.ini`: `Data\interface\SosGui\imgui.ini`;
* [x] [[improvement]] Add `Context` class: hold `popupList` field: All GUI share this `popupList`.