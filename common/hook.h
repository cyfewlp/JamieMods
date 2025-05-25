#ifndef COMMON_HOOK_H
#define COMMON_HOOK_H
#pragma once

#include "common/log.h"

#include <REL/Relocation.h>

namespace LIBC_NAMESPACE_DECL
{
namespace Hooks
{

template <typename func_t>
struct HookData
{
};

template <typename ReturnT, typename... Args>
struct HookData<ReturnT(Args...)>
{
    using func_type = ReturnT(Args...);

    HookData(
        const REL::RelocationID &a_id, const REL::VariantOffset &a_offset, ReturnT (*funcPtr)(Args...),
        bool write6 = false
    )
    {
        m_address = REL::Relocation(a_id, a_offset).address();

        uintptr_t ptr;
        if (write6)
        {
            m_trampoline = SKSE::Trampoline("CallHook6");
            m_trampoline.create(14);
            ptr = m_trampoline.write_call<6>(m_address, reinterpret_cast<void *>(funcPtr));
            ptr = *reinterpret_cast<std::uintptr_t *>(ptr);
        }
        else
        {
            m_trampoline = SKSE::Trampoline("CallHook5");
            m_trampoline.create(14);
            ptr = m_trampoline.write_call<5>(m_address, reinterpret_cast<void *>(funcPtr));
        }
        m_originalFunc = REL::Relocation<func_type>(ptr);
    }

    [[nodiscard]] auto constexpr GetAddress() const -> std::uintptr_t
    {
        return m_address;
    }

    auto operator()(Args... args) const noexcept
    {
        m_originalFunc(args...);
    }

    auto Original(Args... args) const noexcept -> ReturnT
    {
        if constexpr (std::is_void_v<ReturnT>)
        {
            m_originalFunc(args...);
        }
        else
        {
            return m_originalFunc(args...);
        }
    }

    auto ToString() -> std::string
    {
        auto base = REL::Module::get().base();
        return std::format("Hooked at {:#X}, base: {:#X}, offset: {:#X}", m_address, base, m_address - base);
    }

protected:
    std::uintptr_t m_address;

private:
    SKSE::Trampoline                  m_trampoline;
    REL::Relocation<ReturnT(Args...)> m_originalFunc;
};

struct D3DInitHookData : HookData<void()>
{
    explicit D3DInitHookData(func_type *ptr)
        : HookData(REL::RelocationID(75595, 77226), REL::VariantOffset(0x9, 0x275, 0x00), ptr)
    {
        log_debug("Installed {}: {}", __func__, ToString());
    }
};

struct D3DPresentHookData : HookData<void(std::uint32_t)>
{
    explicit D3DPresentHookData(func_type *ptr)
        : HookData(REL::RelocationID(75461, 77246), REL::VariantOffset(0x9, 0x9, 0x00), ptr)
    {
        log_debug("Installed {}: {}", __func__, ToString());
    }
};

struct DispatchInputEventHookData : HookData<void(RE::BSTEventSource<RE::InputEvent *> *, RE::InputEvent **)>
{
    explicit DispatchInputEventHookData(func_type *ptr)
        : HookData(REL::RelocationID(67315, 68617), REL::VariantOffset(0x7B, 0x7B, 0x00), ptr)
    {
        log_debug("Installed {}: {}", __func__, ToString());
    }
};
}
}

#endif // !COMMON_HOOK_H
