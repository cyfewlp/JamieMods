#ifndef HOOKS_HPP
#define HOOKS_HPP

#pragma once

#include "common/config.h"
#include "common/log.h"

#include <cstdint>
#include <type_traits>
#include <vadefs.h>
#include <windows.h>

enum : std::uint8_t
{
    GET_MSG_PROC = 0,
    NUMHOOKS
};

namespace LIBC_NAMESPACE_DECL
{
namespace Hooks
{
template <typename func_t>
    requires requires {
        std::is_function_v<func_t>;
        std::is_pointer_v<func_t>;
    }
class FunctionHook
{
};

class DetourUtil
{
public:
    static auto DetourAttach(void **original, void *hook) -> bool;
    static auto DetourDetach(void **original, void *hook) -> bool;
};

template <typename Return, typename... Args>
class FunctionHook<Return(Args...)>
{
    void *m_originalFuncPtr;
    void *m_hook;
    bool  detoured = false;

protected:
    std::uintptr_t m_address;
    using func_type = Return(Args...);

public:
    explicit FunctionHook(REL::RelocationID id, Return (*funcPtr)(Args...))
    {
        m_address         = id.address();
        m_originalFuncPtr = reinterpret_cast<void *>(m_address);
        m_hook            = reinterpret_cast<void *>(funcPtr);

        if (DetourUtil::DetourAttach(&m_originalFuncPtr, m_hook))
        {
            detoured = true;
        }
    }

    explicit FunctionHook(void *&realFuncPtr, Return (*funcPtr)(Args...))
    {
        m_address         = reinterpret_cast<std::uintptr_t>(realFuncPtr);
        m_originalFuncPtr = realFuncPtr;
        m_hook            = reinterpret_cast<void *>(funcPtr);

        if (DetourUtil::DetourAttach(&reinterpret_cast<PVOID &>(m_originalFuncPtr), m_hook))
        {
            detoured = true;
        }
    }

    ~FunctionHook()
    {
        if (detoured)
        {
            log_debug(
                "Detour {:#x} detach {:#x}",
                reinterpret_cast<std::uintptr_t>(m_hook),
                reinterpret_cast<std::uintptr_t>(m_originalFuncPtr)
            );
            DetourUtil::DetourDetach(&reinterpret_cast<PVOID &>(m_originalFuncPtr), m_hook);
        }
    }

    [[nodiscard]] bool Detoured() const
    {
        return detoured;
    }

    auto ToString() -> std::string
    {
        auto base = REL::Module::get().base();
        return std::format("Hooked at {:#X}, base: {:#X}, offset: {:#X}", m_address, base, m_address - base);
    }

    constexpr Return operator()(Args... args) const
    {
        if constexpr (std::is_void_v<Return>)
        {
            reinterpret_cast<Return (*)(Args...)>(m_originalFuncPtr)(args...);
        }
        else
        {
            return reinterpret_cast<Return (*)(Args...)>(m_originalFuncPtr)(args...);
        }
    }

    constexpr Return Original(Args... args) const
    {
        if constexpr (std::is_void_v<Return>)
        {
            reinterpret_cast<Return (*)(Args...)>(m_originalFuncPtr)(args...);
        }
        else
        {
            return reinterpret_cast<Return (*)(Args...)>(m_originalFuncPtr)(args...);
        }
    }
};

using FuncRegisterClass                              = ATOM (*)(const WNDCLASSA *);
static inline FuncRegisterClass RealRegisterClassExA = nullptr;

// Windows Hook
using MYHOOKDATA = struct _MYHOOKDATA
{
    int      nType;
    HOOKPROC hkprc;
    HHOOK    hhook;
} ATTR_PACKED;

LRESULT CALLBACK MyGetMsgProc(int code, WPARAM wParam, LPARAM lParam);
void             InstallRegisterClassHook();
void             InstallWindowsHooks();

auto WINAPI MyRegisterClassExA(const WNDCLASSA *wndClass) -> ATOM;

}; // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL

#endif
