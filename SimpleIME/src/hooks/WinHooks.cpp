#include "hooks/WinHooks.h"

#include "FakeDirectInputDevice.h"
#include "common/log.h"
#include "detours/detours.h"

#include <dinput.h>

namespace LIBC_NAMESPACE_DECL
{
namespace Hooks
{
void WinHooks::Install()
{
    if (PVOID realFuncPtr = ::DetourFindFunction(MODULE_USER32_STRING, "OpenClipboard"); realFuncPtr != nullptr)
    {
        OpenClipboard = std::make_unique<OpenClipboardHook>(realFuncPtr, MyOpenClipboardHook);
    }

    if (PVOID realFuncPtr = ::DetourFindFunction(MODULE_DINPUT8_STRING, "DirectInput8Create");
        realFuncPtr != nullptr)
    {
        DirectInput8Create = std::make_unique<DirectInput8CreateHook>(realFuncPtr, MyDirectInput8CreateHook);
    }
}

void WinHooks::Uninstall()
{
    DirectInput8Create = nullptr;
    OpenClipboard      = nullptr;
}

BOOL WinHooks::MyOpenClipboardHook(HWND hwnd)
{
    if (g_fDisablePaste)
    {
        return FALSE;
    }
    return OpenClipboard->Original(hwnd);
}

HRESULT WinHooks::MyDirectInput8CreateHook(
    HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter
)
{
    IDirectInput8A *dinput;
    const HRESULT   hresult =
        DirectInput8Create->Original(hinst, dwVersion, riidltf, reinterpret_cast<void **>(&dinput), punkOuter);
    if (hresult != DI_OK)
    {
        return hresult;
    }

    *reinterpret_cast<IDirectInput8A **>(ppvOut) = new FakeDirectInput(dinput);
    return DI_OK;
}
}
}