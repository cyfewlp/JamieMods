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

        void WinHooks::InstallGetClipboardDataHook(const char *funcName)
        {
            log_debug("Install hook: {} {}", MODULE_USER32_STRING.c_str(), funcName);
            if (LPVOID address = ::DetourFindFunction(MODULE_USER32_STRING.c_str(), funcName); address != nullptr)
            {
                GetClipboard = GetClipboardHook(ToUintPtr(address), MyGetClipboardHook);
            }
        }

        using FuncDirectInput8Create = HRESULT (*)(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN);
        static inline FuncDirectInput8Create RealDirectInput8Create = nullptr;

        void WinHooks::InstallDirectInput8CreateHook(const char *funcName)
        {
            log_debug("Install hook: {} {}", MODULE_DINPUT8_STRING.c_str(), funcName);
            /*if (LPVOID address = ::DetourFindFunction(MODULE_DINPUT8_STRING.c_str(), funcName); address != nullptr)
            {
                DirectInput8Create = DirectInput8CreateHook(ToUintPtr(address), MyDirectInput8CreateHook);
            }*/

            auto        pszModule   = "dinput8.dll";
            auto        pszFunction = "DirectInput8Create";
            const PVOID pVoid       = DetourFindFunction(pszModule, pszFunction);
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            RealDirectInput8Create = reinterpret_cast<FuncDirectInput8Create>(pVoid);
            DetourAttach(&reinterpret_cast<PVOID &>(RealDirectInput8Create),
                         reinterpret_cast<void *>(MyDirectInput8CreateHook));
            if (LONG const error = DetourTransactionCommit(); error == NO_ERROR)
            {
                log_debug("{}: Detoured {}.", pszModule, pszFunction);
            }
            else
            {
                log_error("{}: Error Detouring {}.", pszModule, pszFunction);
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
                RealDirectInput8Create(hinst, dwVersion, riidltf, reinterpret_cast<void **>(&dinput), punkOuter);
            if (hresult != DI_OK) return hresult;

            *reinterpret_cast<IDirectInput8A **>(ppvOut) = new FakeDirectInput(dinput);
            return DI_OK;
        }
    }
}