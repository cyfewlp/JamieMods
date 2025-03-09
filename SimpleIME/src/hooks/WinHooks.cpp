#include "hooks/WinHooks.h"
#include "FakeDirectInputDevice.h"

#include "detours/detours.h"

#include <dinput.h>

namespace LIBC_NAMESPACE_DECL
{
    namespace Hooks
    {
        void WinHooks::InstallHooks()
        {
            // InstallGetClipboardDataHook("GetClipboardData");
            InstallDirectInput8CreateHook("DirectInput8Create");
        }

        void WinHooks::UninstallHooks()
        {
            DirectInput8Create = nullptr;
            GetClipboard       = nullptr;
        }

        void WinHooks::InstallGetClipboardDataHook(const char *funcName)
        {
            log_debug("Install hook: {} {}", MODULE_USER32_STRING.c_str(), funcName);
            if (PVOID realFuncPtr = ::DetourFindFunction(MODULE_USER32_STRING.c_str(), funcName);
                realFuncPtr != nullptr)
            {
                GetClipboard = std::make_unique<GetClipboardHook>(realFuncPtr, MyGetClipboardHook);
            }
        }

        using FuncDirectInput8Create = HRESULT (*)(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN);
        static inline FuncDirectInput8Create RealDirectInput8Create = nullptr;

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

        HANDLE WinHooks::MyGetClipboardHook(UINT uFormat)
        {
            if (g_fDisableGetClipboardData)
            {
                return nullptr;
            }
            return GetClipboard->Original(uFormat);
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