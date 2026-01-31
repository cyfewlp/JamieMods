For avoid the inconsistent `textEntryCount`: the `CursorMenu` hide but text entry count not equal zero.
	* When `CursorMenu` is hide and text entry count > 0, try disable IME
	* When `CursorMenu` is show and text entry count > 0, try enable IME


IMenu#ProcessMessage
- called from 82082(sub_140fa3fa0 + 6bd)  , in class UI

## Tool Window

### Controls

- Hided: F2 -> Show;
- Showing: F2 -> Hide;
- Pined: F2 -> Show cursor

### `Console Native UI Menu`

When Open/Close `Steam Overlay` will open/close the "Console Native UI Menu". 
So, we can according to this menu event to disable IME when `Steam Overlay` is showing;

## IMenu

flags:
- `kTopmostRenderedMenu`: Render this menu will prevent others menu render(black background)

---
## TODO: Refactor Input Interception Logic for Skyrim IMenu #status/done #type/refactor
### Context

Following the fix for the message loop bug (see: [[BUGs#FIX Fatal TSF Message Loss & IME Deadlock via Redundant Message Retrieval type/fatal status/done]]), it was confirmed that `ITfKeystrokeMgr` is functioning correctly. The previous "manual" interception of `charEvent` at the engine's lower level is redundant and prone to state-mismatch errors.

- [[SimpleIME/src/menu/ImeMenu.cpp|ImeMenu]]
- [[SimpleIME/src/ImeWnd.cpp|ImeWnd]]
### Objective

Simplify the input flow by routing all character input through the TSF system and only "compensating" the game engine with processed characters.
### Refactoring Plan

- [ ] **Remove Legacy Interception:** Strip out the complex, manual `charEvent` filtering logic within the `IMenu` hooks.
- [ ] **Conditional Interception:** - When `SimpleIME` is **Active** (TSF context is focused and non-ENG keyboard is selected): - Silence/Intercept all raw `charEvent` signals sent to the underlying `IMenu`.
- [ ] **Implement Compensation Logic:**
    - Forward `WM_CHAR` messages to `IMenu` **only if**:
        1. The IME is active but in **English/Direct Input mode**.
        2. The keystroke was **not eaten** by the TSF `KeystrokeMgr` (e.g., control keys, function keys, or bypass combinations).
        3. The character is a result of a completed IME composition (captured via `ITfTextEditSink` or `WM_CHAR` post-filtering).
### Expected Benefits
1. **Simplified State Management:** No longer need to manually track if the user is "typing" vs "navigating."
2. **Zero Input Loss:** Ensures that "English-mode" typing within a Chinese IME behaves exactly like a native English keyboard.
3. **Improved Compatibility:** Fixes edge cases where Shift-combinations or Alt-codes were being dropped or double-processed.

