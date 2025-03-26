#include "hooks/WinHooks.h"
#include "FakeDirectInputDevice.h"
#include "common/log.h"

#include "detours/detours.h"

#include <dinput.h>

namespace LIBC_NAMESPACE_DECL
{
    namespace Hooks
    {
        void WinHooks::InstallHooks()
        {
            InstallGetClipboardDataHook("OpenClipboard");
            InstallDirectInput8CreateHook("DirectInput8Create");
        }

        void WinHooks::UninstallHooks()
        {
            DirectInput8Create = nullptr;
            OpenClipboard      = nullptr;
        }

        void WinHooks::InstallGetClipboardDataHook(const char *funcName)
        {
            log_debug("Install hook: {} {}", MODULE_USER32_STRING.c_str(), funcName);
            if (PVOID realFuncPtr = ::DetourFindFunction(MODULE_USER32_STRING.c_str(), funcName);
                realFuncPtr != nullptr)
            {
                OpenClipboard = std::make_unique<OpenClipboardHook>(realFuncPtr, MyOpenClipboardHook);
            }
        }

        void WinHooks::InstallDirectInput8CreateHook(const char *funcName)
        {
            log_debug("Install hook: {} {}", MODULE_DINPUT8_STRING.c_str(), funcName);
            if (PVOID realFuncPtr = ::DetourFindFunction(MODULE_DINPUT8_STRING.c_str(), funcName);
                realFuncPtr != nullptr)
            {
                DirectInput8Create = std::make_unique<DirectInput8CreateHook>(realFuncPtr, MyDirectInput8CreateHook);
            }
        }

        constexpr auto WinHooks::ToUintPtr(LPVOID ptr) -> std::uintptr_t
        {
            return reinterpret_cast<std::uintptr_t>(ptr);
        }

        BOOL WinHooks::MyOpenClipboardHook(HWND hwnd)
        {
            log_debug("MyOpenClipboardHook ");
            if (g_fDisablePaste)
            {
                return FALSE;
            }
            return OpenClipboard->Original(hwnd);
        }

        HRESULT WinHooks::MyDirectInput8CreateHook(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut,
                                                   LPUNKNOWN punkOuter)
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