#pragma once

#include "common/log.h"
#include "hooks/Hooks.hpp"

#include <Unknwnbase.h>
#include <guiddef.h>

namespace LIBC_NAMESPACE_DECL
{
namespace Hooks
{
class OpenClipboardHook : public FunctionHook<BOOL(HWND)>
{
public:
    explicit OpenClipboardHook(void *realFuncPtr, func_type *ptr) : FunctionHook(realFuncPtr, ptr)
    {
        log_debug("Installed {}: {}", __func__, ToString());
    }
};

class DirectInput8CreateHook : public FunctionHook<HRESULT(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN)>
{
public:
    explicit DirectInput8CreateHook(void *&realFuncPtr, func_type *ptr) : FunctionHook(realFuncPtr, ptr)
    {
        log_debug("Installed {}: {}", __func__, ToString());
    }
};

class WinHooks
{
    static inline std::unique_ptr<OpenClipboardHook>      OpenClipboard      = nullptr;
    static inline std::unique_ptr<DirectInput8CreateHook> DirectInput8Create = nullptr;

    static inline std::atomic_bool g_fDisablePaste       = false;
    static constexpr const char   *MODULE_USER32_STRING  = "User32.dll";
    static constexpr const char   *MODULE_DINPUT8_STRING = "dinput8.dll";

public:
    static void Install();

    static void Uninstall();

    static void DisablePaste(bool disable)
    {
        g_fDisablePaste = disable;
    }

private:
    static BOOL    MyOpenClipboardHook(HWND hwnd);
    static HRESULT MyDirectInput8CreateHook(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN);
};
}
}