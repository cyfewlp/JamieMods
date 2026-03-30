## [unreleased]

### 🚜 Refactor

- *(imguiex)* Now `ErrorNotifier` no `imguiex_m3` dependendcy

### 📚 Documentation

- Add CHANGELOG


## [0.1.0] - 2026-03-30

### 🚀 Features

- *(SimpleIME/ImeMenu)* Optimize the mouse event capture logic
- *(SimpleIME/ImeUI)* WIP: support preview installed font;
- *(SimpleIME/ImeUI)* WIP: support set default font from preview font;
- *(SimpleIME/ImeUI)* WIP: optimize FontBuilder; move all operations about modify font to NewFrame to avoid crash; consume all ui event when ToolWindowMenu is showing
- *(SimpleIME/ImeUI)* WIP: optmize PreviewFont
- *(SimpleIME/ImeUI)* WIP: checkout ImGui to laster master branch; Remove code specific to the docking branch
- *(SimpleIME/ImeUI)* WIP: refactor PreviewFont & FontBuilder
- *(SimpleIME/ImeUI)* WIP: By set FontDataOwnedByAtlas to false to avoid copy preview font data during merging.
- *(Simple/ImeUI)* Support save the FontBuilder build result. Optimize FontBuilder structure.
- *(Simple/ImeUI, SimpleIME/Configs)* Try to use themeIndex restore theme during init; save themeIndex
- *(SimpleIME)* Add DebounceTimer: debounce for font preview.
- *(SimpleIME/Fonts)* Draw search box with a search icon(Channels API)
- *(SimpleIME/Fonts)* Add the `ScrollY` flag to keep search box always visible;
- *(SimpleIME/Fonts)* Make AddFont button more eye-catching
- *(SimpleIME/Fonts)* Show "preview default font" tips.
- *(SimpleIME/Fonts)* Add some padding to table and file icon
- *(SimpleIME/Fonts)* Add State enum class to show previewPanel state;
- TextField: support the `outlined` style.
- Small tool to auto extract m3 spec data.
- Use the approximation SRGB <-> Linear convertion function to fix `BlendState`.
- Support generate struct from `TokenSet`
- Add menuitem components and Begin/EndMenu; Add menu/TextField specs; TextField support readonly pattern
- Refactor text field implementation to support leading and trailing icons; introduce TextFieldContent struct
- Click `MenuItem` will auto-close current popup.
- Introduce new button and icon button specifications; Reduce/Remove template functions with different colors schema components.
- Add Floating Action Button (FAB) specifications and button group configurations; add new Python script & cmake command to build only used icon to TTF; Remove nerdfont
- Add dependency on icons target in CMake for improved build process
- Limit floating toolbar position within screen margins(see ToolBar specs)
- Refactor TextField config param; Fix: Button/TextField/NavItem correct handle the "##" in label(hide text after this).
- Button support elevated colors: support toggle variant.
- *(ErrorNotifier)* Enhance error display with M3 styles and improve message rendering
- *(SimpleIME/LanguageBar)* Remove unused type aliases
- Update versioning to include pre-release label and improve display format; re-write NexusMod mod main page
- Add GitHub Actions workflow for rolling beta release automation
- Set up CMake 4.x in GitHub Actions workflow
- Skip benchmark test if no sources;
- *(m3)* Introduce SameLine to support auto-scale size params;  fix(SimpleIME): Fix ImeWindow 'ENG' the no center align issue
- *(ime)* Improve Japanese IME mode tracking and keyboard sync (#8)
- *(github-actions)* Add release date to generated release notes
- *(SimpleIME/LanguageBar)* Supports switch to default english input method.
- *(ErrorNotifier)* Allow move window; push text color for "close" button;
- *(SimpleIME/FontManager)* Auto-detect system default UI font (ref #13)
- *(SimpleIME/ToolWindowMenu)* Support UserEvents; refactor ImeOverlay/ToolWindow lifecycle
- *(SimpleIME/ImeMenu,ToolWindowMenu)* Only block user input when ToolWindow showing
- *(SimpleIME,Overlay)* Add auto toggle language bar setting and functionality (ref #15)
- *(imguiex)* Only support imgui v1.92.6
- *(cmake,SosGui)* Add add_build_lucide_icons cmake function to repacke lucide icons from a icon list file;
- Merge sosgui

### 🐛 Bug Fixes

- Don't install ScaleformHooks if AlwaysActiveIme is true
- Use GFxMovieView#handleEvent handle paset text; send to TopMenu will cause console menu infinite loop process message;
- Update ConsoleProcessMessageHook se id; OnTextEntryCountChanged support 1.5.97
- *(SimpleIME/ImeUI)* Configure ImGuiStyle after apply theme correctly;
- *(SimpleIME/ImeUI)* Support save fontSize correctly;
- *(Simple/ImeUI)* Revert the cursor blink change;
- *(Simple)* Fix the refrences that moved Settings memebers;
- *(SimpleIME/Fonts)* By call `BringWindowToDisplayFront` to keep IME window always in topmost;
- This SKSE_Scaleform fn return value is meaningless. Should read from `ControlMap`
- M3Styles#currentScale default value now is 0.0F: avoid `UpdateScaling` early out.
- LanguageBar: OPEN_SETTINGS only set once;
- Add Tooltips spec; Fix incorrect child flag parameter pass in `BeignList`
- Use `ItemHoverable` check hovered state during active text-field logic.
- *(M3/Icon)* Apply the limitation: min layout size
- *(M3/SimpleIME/ImeUI)* Correct indentation for EndNavRail call
- *(SimpleIME)* Fix GFxCharEvent memory leak in ImeMenu
- *(FontManager)* Add FIXME to investigate performance of font query system; add FIXME for IMM32 composition issue
- Update legal copyright to MIT License; adjust CPack output settings and add dist to .gitignore; update fmod info
- *(SimpleIME)* Apply RGB mask to theme source color; update configuration documentation
- *(github-actions)* Add FontForge to env:path
- *(SimpleIME/Imm32)* Add missing MarkDirty call after composition text change;
- *(cmake)* Fix the faulty MATERIAL_YOU_PATH var;
- *(cmake)* Add the missing build presets
- *(cmake)* Fix incorrect installation action for lucide license
- *(SimpleIME/ImeMenu)* Add AllowSaving flag to avoid can't save game
- *(SimpleIME/AppearancePanel)* Addd AlphaOpaque to disable alpha preview;
- *(SimpleIME/ImeWindow)* Avoid change ime window position if no pos update policy;
- *(github-actions)* Tag v2.0.0-beta -> SimpleIME-v2.0.0-beta
- *(SimpleIME/InputFocusAnchor)* Fix  the unsigned index overflow error in loop;
- *(imguiex/FontScope)* Remove `FontScope` data access functions: M3Styles may return empty fontscope.
- *(SimpleIME)* Construct ImeOverlay if request show;
- *(imguiex/M3)* Default ui scale is 1.0F now

### 💼 Other

- Integrate ImTheme. parse toml and load as ImGUi theme
- Support sync EnableIme, upgrade and fix bug for IsEnable(now IsWantCaptureInput)
- Disable send `delete` event to ImGui: Hook AllowTextInput to  fix `RaceMenu/Console` text entry can't use IME
- Update mod page;  ErrorNotifier moved to common
- Optimize window pos update logic
- Remove hwndGame field; fix: `AddTask`, `ImeWnd::Focus` incorrect return value
- Add ImeManagerComposerTest; optimize class dependencies
- Add Settings; Remove ImeUIWidgets;
- Update default english translation
- Add `IsShouldEnableIme` to correctly enable ime when checked "KeepImeOpen" or exists text entry;
- Add ToString() for FunctionHooks, HookData; modify UiHooks and ScaleformHooks to singleton
- Bump to version 1.2.0
- Discard keydown/up event when message filter is enabled;
- Re-layout settings panel by TabBar
- Skip load failed IME
- Support close message filter once manually
- Draw caret by TextEditor acp selection; try to enable DPI aware in IME thread;
- Support config/save Font_Size
- Remove theme files; update fmod & cpack
- Supports select a candidate by click; deprecated config Highlight_Text_Color
- Stuff
- Prepare release 1.2.1
- Bump to version 1.3.0
- Remove UiHooks; remove message filter related UI elements
- Comment D3d and event hook
- Tier1: fix bug that Imm32TextService eaten messages it didn't process. ; remove ITfKeystrokeMgr
- Imm32TextService: by call ImmAssociateContext to enable/disable ime message when focus changed;  render all states key in debug;
- Fix bug that MsPY candidate list update error in some cases;TextEditor: Collapse selection after inertText; ImeUI: fix bug that the composition strings render incorrectly;
- ToolWindowMenu now use game cursor to avoid the FixInconsistentTextEntryCount is called
- AbortIme if click area not ImeMenu;Call CloseCandidate when end composition if candidate is choosing;
- Remove focus type module; Check "SimpleIME/docs/BUGs.md"
- Pass 2nd parameter with false to disable commonlib init spdlog
- No longer show ImGui cursor; destroy HIMC correctly;
- *(SimpleIME/ImeUI)* Avoid IME UI render area exceeds game window size and overlap text entry; Fix the charBoundaries calculate logic
- *(SimpleIME/ImeUI)* Support preview text with configured fontSize
- *(SimpleIME/ImeUI)* Support ime in ToolWindow and consume char event when ToolWindow is showing; scaleform event handleer function return UI_MESSAGE_RESULTS now;
- *(SimpleIME/ImeUI)* Add help & warning modal
- Refactor configs module; remove AppConfig; Move all the configuration to the Settings;
- *(Simple/ImeUI)* Use the gear icon to replace settings's checkbox;
- *(Simple/ImeUI)* Move the class FontBuilder/View to singleton file; Call AllowTextInput to receive CharEvent;
- Move `FontManager.cpp` to dir `ui/fonts`
- Style: Add ToolBar to combine font builder all interact buttons
- *(SimpleIME/Fonts)* By use the Table `SizingStretchProp` flag to right align a group(ToolBar);
- Add navigation rail item support;
- Remove Tsf/globalhooks test
- SimpleIME: Icon fonts are no longer automatically merged into the default font.
- Only first TranslatorHolder##RequestUpdateHandle caller can modify the shared Translator
- Remove `GameProfileSwitch.ps1` and replaced by python script.
- Add State machine;remove `context`; ImeMenu/ToolWindowMenu will early out if ImeApp is not initialized.
- Print log to debugger when spdlog is uninstalled
- Move ImeUi to ui package
- Add `ImeWindow`: responsible for drawing IME and automatic layout.
- Settings: Add zoom config; remove `font_size_scale` config;
- Settings: Add zoom config; remove `font_size_scale` config;
- Intercept all CharEvnet if IM activated. And send all `ImeWnd` received WM_CHARS message to game.
- Reduce IME window comp/cand padding
- Shutdown spdlog on dll detach
- Theme Builder
- Theme Builder(Adjust styles)
- Button disabled style
- Try-catch Draw: shutdown if occur any unexpected exception during `Draw()`;
- Optimize M3Styles init flow
- UI layer no longer dependency M3Styles;
- Fix/update ControlMap#ToggleControls call; small fixes;
- Refactor ScaleformHook
- InputFocusAnchor: fix pass value param boundaries incorrect.
- Remove  the HWND paramter from `CommitCandidate`
- Colors add two helper functions; And replace all Surface#hoverd/pressed functions call.
- `ImeController`: Add CommitCandidate method; add Shutdown/IsReady function;
- Adjust LanguageBar style; modify `fmod` install step and remove `ImThemes.png`;
- Move FontPreviewPanel implementation to separate file
- *(SimpleIME)* Support DPI awareness and refactor DPI state
- Remove and replace Surface/ContentToken -> `ColorRole`;
- ColorRole moved to namespace Spec;
- *(ime)* M3 UI redesign & flicker-free candidate buffering**
- Renamed TextStore::GetText parameters to clarify. more restrict acpStart/End validate.
- *(SimpleIME)* M3 Scheme ptr moved to AppearancePanel as member from local static prt;  rename members;
- *(SimpleIME)* Avoid send some document control char to game;
- Refactor CMake/vcpkg layout for multi-project workspace
- Move assets/lucide icons into SimpleIME.
- Common/imguiex -> imguiex/imguiex
- Add cmake/material-you subproject to replace the extern for MaterialYou lib;
- Merge multiple lines after one bullet;
- *(SimpleIME/InputFocusAnchor)* Introduce InvalidateCachedMenuIndex fun and called from MenuOpenClose event callback;   Add ImeWnd thread description
- Improve Mod main page, SimpleIME demo; remove deprecated configuration;
- Fix ToggleMenuModeContextIfNeed bug.
- Sync default configuration;
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

- *(SimpleIME/Fonts)* Refactor FontBuilder layout; fix: clear usedFontId vector when reset;
- EventHandler
- TranslationLoader. The main section parameter is no longer require.
- Rename imgui to imguiex and update M3 TextRole
- *(SimpleIME)* Rename ImGuiManager -> imgui_system; All member static functions ->  free functions
- Update StyleGuard usage to use template methods for style and color settings
- Refactor `NavItem`
- Use M3::BeginCombo & M3::MenuItem refactor zoom list;
- *(SimpleIME/State)* Optimize state management using bitwise operations
- *(SimpleIME/TextStore)* Rename parameters for clarity and consistency
- *(SimpleIME)* Transition to RAII-based UI lifecycle and simplify Translator architecture
- *(SimpleIME)* Use raw pointer for global translator to avoid destruction order issues
- *(States)* Rename SetTsfFocus to FocusTextService; restyle `states` UI;
- *(SimpleIME/LanguageBar)* Re-layout the font builder
- *(SimpleIME/FontBuilder)* Downgrade some class-static functions to free functions in source file;
- *(ImeManager)* Remove outdated TODO comment; add FIXME in LanguageBar
- *(SimpleIME)* Move himc set/restore logic to the ImeController(closes #6)
- *(SimpleIME/ImeMenu)* Remove kImeCharEvent validate in IMM32 todo comment;  Fix clang-tidy warnings; Relase static functions to anonymous namespace free funtions;
- *(State)* Change m_state to atomic for thread safety
- *(SimpleIME/ImeWindow)* Remove 'IsNeedRelayout' flag to improve positioning
- *(SimpleIME)* The frame profiler moved to ImeMenu Postdisplay
- *(SimpleIME)* Fix DEBUG macro usage to _DEBUG; move IME states drawing to separate window.
- *(SimpleIME)* Only block events when ToolWindow showing
- *(SimpleIME/TSF)* Deprecate ITfKeystrokeMgr and ITfMessagePump(See ImeWnd::TsfMessageLoop comments)
- *(SimpleIME/Focus)* Refactor focus system: Only guarantee the minimum state requirements of the mod core.
- Fix all build errors
- *(SimpleIME)* Move FontManager to common module; move header-only lib to extern directory;
- *(SimpleIME)* Move the imgui_system to common lib imgui_manager
- *(SosGui)* Remove font manager module; Migrate to common FontManager; Migrated to imgui_manager
- *(SosGui)* Remove About/SettingsPopup; Migrated to lucide icons;
- *(SimpleIME)* Migrate translator_manager to the common module;
- *(SosGui)* Depecated translator module and Migrate to i18n::Translator(WIP)

### 📚 Documentation

- Integrate Doxygen and add doxygen-awesome-css submodule for enhanced documentation styling
- *(README)* Update project description and enhance build instructions; add features and requirements sections
- *(SimpleIME)* Fill CHANGELOG for v2.0.0-beta release
- Update CMake preset names for SimpleIME build instructions
- Add SimpleIME's nexus mod articles;
- Update doxygen input source; Update nexus docs style and explained the event intercept mechanism.

### 🎨 Styling

- Update .clang-format to allow alignment across comments and adjust short function settings

### 🧪 Testing

- *(Simple)* Add ConfigSerializerTest; remove unused test
- Add TestEditorTest

### ⚙️ Miscellaneous Tasks

- Remove SimpleIME, now lives in its own repo
- Update README
- Remove sosgui,simpleIME
