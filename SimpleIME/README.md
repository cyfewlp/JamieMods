## What

Make SkyrimSE supports IME. Use ImGui + Win32 + IMM to do this.

## Impletion

1. create a invisible child window override SkyrimSE window;
   Because wo need a Unicode window.
2. Always focused the child window;
   Snap grab input focus and handle it in child window message loop;
3. Hook `GetMsgProc` and convert `WM_IME_COMPOSITION` message to our custom message WM_CUSTOM_IME_COMPOSITION
to avoid SkyrimSE default message loop to destroy `WM_IME_COMPOSITION` message(`PeekMessage` & `DispatchMessageA`);