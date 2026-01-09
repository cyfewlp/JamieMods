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

## Select candidate manually in Imm32 #status/Investigating#type/bug

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

