## What
Make SkyrimSE/AE supports IME(Input Method Editor). Mainly for Chinese players. Use ImGui + Win32 + IMM to do this.

## Implement
1. create a invisible child window override Game window;
   Because wo need a Unicode window. Default Skyrim create a ANSI window(CreateWindowExA);
2. Always focused the child window;
     Snap grab input focus and handle it in child window message loop; This logic will not effect(atleast in my test case) game input event 
3. Hook `GetMsgProc` and convert `WM_IME_COMPOSITION` message to our custom message WM_CUSTOM_IME_COMPOSITION.
Because we need avoid to Skyrim default message loop to destroy `WM_IME_COMPOSITION` message(`PeekMessage` & `DispatchMessageA`);
4. Overvide System IME's composition & candidate window and use ImGui render it.