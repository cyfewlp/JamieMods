#pragma once

#include "common/log.h"
#include "hooks/Hooks.hpp"

namespace LIBC_NAMESPACE_DECL
{
    namespace Hooks
    {
        class GetClipboardHook : public FunctionHook<HANDLE(UINT)>
        {
        public:
            explicit GetClipboardHook(void *realFuncPtr, func_type *ptr) : FunctionHook(realFuncPtr, ptr)
            {
                log_debug("{} hooked at {:#x}", __func__, m_address);
            }
        };

        class DirectInput8CreateHook : public FunctionHook<HRESULT(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN)>
        {
        public:
            explicit DirectInput8CreateHook(void *&realFuncPtr, func_type *ptr) : FunctionHook(realFuncPtr, ptr)
            {
                log_debug("{} hooked at {:#x}", __func__, m_address);
            }
        };

        class WinHooks
        {
            static inline std::unique_ptr<GetClipboardHook>       GetClipboard       = nullptr;
            static inline std::unique_ptr<DirectInput8CreateHook> DirectInput8Create = nullptr;

            static inline bool        g_fDisableGetClipboardData = false;
            static inline std::string MODULE_USER32_STRING       = "User32.dll";
            static inline std::string MODULE_DINPUT8_STRING      = "dinput8.dll";

        public:
            static void InstallHooks();

            static void UninstallHooks();

            static void DisableGetClipboardData(bool disable)
            {
                g_fDisableGetClipboardData = disable;
            }

        private:
            static void           InstallGetClipboardDataHook(const char *funcName);
            static void           InstallDirectInput8CreateHook(const char *funcName);
            static constexpr auto ToUintPtr(LPVOID ptr) -> std::uintptr_t;

            static HANDLE  MyGetClipboardHook(UINT uFormat);
            static HRESULT MyDirectInput8CreateHook(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN);
        };
    }
}