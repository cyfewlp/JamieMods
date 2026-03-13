# Changelog

A condensed history of significant changes to SimpleIME.

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
