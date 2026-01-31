## Bug with temporary focus #status/done #type/bug

 ~~can't input. May missing IMM32 context?~~

Already remove focus type system.
## Removal of the Focus Management System #status/done #type/bug

* Binding the unlock/restore keyboard operations to mod enable/disable operations.
* If we do this, the Permanent/Temporary focus will become unnecessary;

The **Focus Management System** has been completely deprecated and removed from the codebase. This module was identified as a primary source of instability due to threading conflicts and side effects caused by DirectInput state manipulation.
### Technical Reasoning

- **Thread Synchronization Issues:** The system previously managed two distinct focus types—**"Permanent"** and **"Temporary"**—which operated across different threads. Managing keyboard state transitions (locking and restoring) between these threads led to unpredictable behavior and race conditions.
    
- **DirectInput Interference:** The core functionality relied on `SetCooperativeLevel` to yield or regain keyboard control. We discovered that any call to `SetCooperativeLevel` triggers a reset of the DirectInput device state. This disruption interferes with the game engine's input buffer, specifically breaking the toggle logic of the **Skyrim Console Menu**.
    
- **Console Menu Interruption:** A significant side effect was that the first attempt to open or close the Console would be interrupted or ignored if a `SetCooperativeLevel` call occurred during the input sequence. This made the "Temporary" focus feature practically unusable and counter-productive.
### Final Resolution

- **System Deprecation:** All logic related to "Temporary" focus and dynamic cooperative level switching has been stripped out.
    
- **Simplified Logic:** The implementation now exclusively uses the "Permanent" focus model, ensuring a stable and consistent connection to the game's input stream.
    
- **User Workflow Change:** For users who previously relied on "Temporary" focus to unlock the keyboard for other applications, we recommend using the standard **"Enable/Disable Mod"** toggle. This provides the same functional outcome without compromising the stability of the DirectInput queue.

## Select candidate manually in Imm32 #status/investigating #type/bug

Ime::Imm32::Imm32TextService::CommitCandidate

```c++
HIMC hImc = ImmGetContext(hwnd);  
  
bool result = true;  
if (hImc)  
{  
    result = ImmNotifyIME(hImc, NI_SELECTCANDIDATESTR, 0, index) != FALSE;  
}  
  
ImmReleaseContext(hwnd, hImc);
```
The  `NI_SELECTCANDIDATESTR` flag does not work as expected.

## Font configruation incorrectly #status/done #type/bug 

* Add new method: `Ime::ImeUI::FillCommonStyleFields`. This method will set common fields after apply theme.
* Remove `ImGuiStyle` configuration code from `ImeWnd::InitImGui`(already moved to `ImeUI::FillCommonStyleFields`).

## Missing int type converter #type/bug #status/done 

## IME Window drawing behind with ToolWindow #status/done 

- Must Keep IME window is the topmost window;
	- Use this call:  `ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());`
## IME: composition result Destination incorrect #type/bug #status/todo 
## IME: `CharEvent` Intercept behaviour incorrect #type/bug #status/todo 

## FIX Fatal: TSF Message Loss & IME Deadlock via Redundant Message Retrieval #type/fatal #status/done 
### Description

A critical flaw existed in the application's message loop where input messages (notably from the Microsoft Pinyin IME) were being discarded. This occurred because the loop invoked both `ITfMessagePump::PeekMessageW` and the standard `GetMessageW` in a way that the latter would overwrite the message buffer already populated by the former.

When using **Microsoft Pinyin**, certain state-change triggers (like the **Shift** key for Chinese/English toggling) rely on a continuous sequence of messages. Discarding a single `WM_KEYUP` or `WM_IME_NOTIFY` message would cause the IME's internal state machine to hang, resulting in a complete loss of keyboard input responsiveness.
### Technical Analysis

The bug followed this logic flow:

1. `pMessagePump->PeekMessageW(..., PM_REMOVE, &fResult)` was called, successfully pulling a message from the queue.
    
2. If `fResult` was `FALSE` (meaning TSF didn't internally "eat" the message), the code proceeded to call `GetMessageW`.
    
3. `GetMessageW` is a blocking call that waits for the _next_ message, immediately overwriting the `MSG` structure containing the previously peeked message.
    
4. **Result:** Every message that TSF didn't explicitly handle was effectively deleted from the system without being dispatched via `TranslateMessage`/`DispatchMessage`.
### Impact
- **IME Deadlock:** Microsoft Pinyin would get stuck in a "pending" state during composition.
- **Input Blocking:** The `Shift` key would fail to toggle modes after the first few uses.
- **UI Unresponsiveness:** Systematic loss of `WM_PAINT` or other critical window messages.
### Resolution

The message loop was refactored to use a **Single-Source-of-Truth** pattern:
- Implemented a non-blocking `PeekMessageW` loop that processes the message immediately if TSF returns `fResult == FALSE`.
- Integrated `MsgWaitForMultipleObjectsEx` with the `MWMO_INPUTAVAILABLE` flag to ensure the thread sleeps efficiently without high CPU usage while remaining responsive to all input types.
- Guaranteed that every message removed from the queue is either consumed by `ITfKeystrokeMgr` or dispatched to the window procedure.

Corrected Logic Pattern
```cpp
while (pMessagePump->PeekMessageW(&msg, ..., PM_REMOVE, &fResult) == S_OK) {
    if (fResult) continue; // Handled by TSF
    if (!HandleKeystroke(msg)) { // Handled by KeystrokeMgr
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}
// Thread-safe sleep until next message
MsgWaitForMultipleObjectsEx(0, nullptr, INFINITE, QS_ALLINPUT, MWMO_INPUTAVAILABLE);
```