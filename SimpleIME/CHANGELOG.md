# Changelog

A condensed history of significant changes to SimpleIME.

---

## 2.0.0-beta

### Breaking Changes
- **Focus Mode removed** — The Permanent / Temporary focus-mode system has been
  entirely removed. To disable IME input, disable the mod via its toggle; the
  mod will stop intercepting events and yield control back to the game.
- **Config keys removed** — `FocusType`, `PermanentFocusManager`-related keys
  are no longer read.

### Core
- **IMenu-based charEvent interception** — Replaced the `DispatchInputEvent`
  global hook (v1.3.0) with `RE::IMenu` at depth-priority 13. `ImeMenu` now
  sits in the menu stack and intercepts every `GFxCharEvent` while IME
  composition is active. Characters that should reach the game are
  re-dispatched as a custom `kImeCharEvent` scaleform event type — a value not
  used by the vanilla game — so they bypass the interception guard and arrive
  at the underlying menu cleanly, with no global hook required.
  Also removes `AddMessageHook` and the old `UiHooks` plumbing.
- **Flicker-free candidate buffering** — Double-buffered candidate list to fix
  Microsoft Pinyin (MSPY) one-frame blink. TSF/IMM32 writes to a write-buffer;
  the render thread reads from a read-buffer, synchronised via
  `UpdateCandidateUiIfDirty`. `EndUIElement` no longer clears the cache,
  preventing blinks during MSPY state transitions.

### Bug Fixes
- **GFxCharEvent memory leak** — `BSUIScaleformData`'s destructor does not
  free `scaleformEvent`. ImeMenu now collects `kImeCharEvent` pointers during
  `ProcessMessage` and frees them in `PostDisplay` via `RE::GMemory::Free`.
  Safe because `PostDisplay` always runs after `ProcessMessage` on the same
  render thread within a given frame.
- **LANG_PROFILE_ACTIVATED mis-retention** — `RefreshProfiles` no longer
  auto-inserts `DEFAULT_LANG_PROFILE`; `OnCharEvent` is gated on
  `!ImeDisabled()` in addition to `LANG_PROFILE_ACTIVATED`.
- **imgui.ini location** — ImGui layout is now saved to the plugin interface
  directory so it persists across game launches and mod updates.
- **ToolWindow shortcut** — Use `IsKeyChordPressed` to avoid routing through
  ImGui focus (call-site no longer needs to belong to an ImGui window).
- **Translator lifetime** — Switched to `static inline` instance to guarantee
  `ImeWnd` is destroyed before the translator.

### Features
- **Material Design 3 UI** — Full MD3 component library: AppBar, Navigation
  Rail, Floating Toolbar, Dialog, Chip / ChipGroup, ListItem, FAB, SearchBar,
  TextField (outlined & filled), CheckBox, Divider, and more. ImeWindow
  migrated to MD3 components; popup positioning uses
  `ImGui::FindBestWindowPosForPopupEx`.
- **Font Builder** — Build and preview custom icon fonts in-game. Nerd Font
  dependency removed; replaced with a minimal repackaged Lucide TTF containing
  only the icons actually used, generated at CMake configure time via
  `fontforge` + Python.
- **Theme Builder** — HCT / M3 tonal-palette color-scheme editor with
  live preview and light/dark theme toggle.
- **New config: `fixInconsistentTextEntryCount`** (default `true`) — Corrects
  text-entry-count state mismatch that could leave IME enabled when no input
  field is focused.
- **RAII UI lifecycle** — Translator and ImeWnd teardown order is now safe;
  no destruction-order crashes on game exit.

### Refactoring
- **`Configuration` struct** — Decouples raw TOML field names from the runtime
  `Settings` struct. Converters synchronise between the two; raw TOML keys no
  longer pollute application code.
- **`InputFocusAnchor`** — Replaced menu-stack iteration with a focused-item
  search (`m_lastFocusMenuIndex` for O(1) updates). Removed the
  `ComputeScreenMetrics` scaleform hook (called directly from ImeWindow).
- **`WCharUtils`** — Converted from static-method class to a namespace of free
  functions; only `std::wstring_view` overloads kept.
- **`ImeApp` cleanup** — `g_settings` moved into `ImeApp`; `D3DInitHook`
  renamed to `g_D3DInitHook`; dead/unused hook registrations removed.

---

## 1.3.0
- Font size configurable in Settings panel.
- Full `ImThemes` support; deprecated obsolete color names (`TabActive`, `TabUnfocused`, etc.).
- Caret display in IME window; new `KEYBOARD_OPEN` state flag for Japanese IME.
- Fix: clear composition/candidate/alphanumeric state on input-method switch.
- Fix: discard game keydown/up events when message filter is active.

## 1.2.0
- Fix: `Enable Mod` toggle.

## 1.1.2
- Save and restore UI configuration.
- Message notify window.
- `FocusType` refactored to enum class.
- Various bug fixes: duplicate `PopAndPushType` call, `SendNotifyMessage` return-value check, suppress error messages while mod is disabled.
- Deprecated config keys: `UI#Default_Theme`, `UI#Default_Language`, `General#Enable_Unicode_Paste`.

## 1.1.0
- Modex: disable forwarding `delete` event to ImGui.
- Fix `RaceMenu` text-entry IME support via `AllowTextInput` hook (also fixed Console).
- Dynamic caret screen-position detection with scroll support.

## 1.0.0
- Public API for third-party mod integration (`IME_INTEGRATION_INIT`, `IME_COMPOSITION_RESULT`).
- Unicode paste via `ProcessMessage` hook.
- Fix: IME state inconsistency when using `Keep Ime Open` with Temporary focus.
- Fix: TSF message-pump bug causing Microsoft Pinyin deadlock on Shift toggle.

## 0.2.0
- Introduced `PermanentFocusManager` and `SafeImeManager` strategies; default is `PermanentFocusManager`.

## 0.1.0
- Refactored focus and `IME_DISABLED` state management (no longer associate empty document with TSF).
- Multi-language support (default: English and Chinese).

## 0.0.8
- Settings window.
- FMOD / RaceSexMenu patch detection.
- ImGui theme loaded from file (`ImThemes` format).
- Config: `keepimeopen` replaces `Always_Enable_Ime`.

## 0.0.7
- SteamOverlay: open/close Console Native UI Menu used to detect overlay state and disable IME accordingly.
- Abort IME composition when ImGui window loses focus.
- Make composition and candidate windows follow cursor on first appearance.

## 0.0.6
- Dynamic IME enable/disable: disable when no menu with a text field is open.
- Iterate menu stack to detect active input context.
- Enable when game cursor is visible.

## 0.0.5
- Config refactor; `Property` class optimised; switched back to SimpleINI.
- TSF candidate support added; TSF/IMM32 switchable via config.
- Fix: candidate update for Microsoft PinYin.
- Fix: crash when comp & candidate UI were open on game reload.
- WcharBuf optimised and covered by unit tests.

## 0.0.4
- Fix: crash on game close with Japanese IME open.
- Fix: Modex crash via `ImGuiIO::ClearInputKeys` (delayed `SetFocus` call).
- ImeWnd / ImeUI launched in standalone thread with own message loop to avoid COM `COINIT_MULTITHREADED` conflicts.

## 0.0.3
- Initial working release: DirectInput hook, TSF candidate list, tool-window auto-hide on focus loss.
- UI color configuration.
- Fix: IMGui cursor dead; fixed memory leaks; improved log source location.
