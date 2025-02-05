#ifndef HOOKS_HPP
#define HOOKS_HPP

#pragma once

#include "Configs.h"
#include <REL/Relocation.h>
#include <cstdint>
#include <windows.h>

enum : std::uint8_t
{
    GET_MSG_PROC = 0,
    NUMHOOKS
};

#define MAKE_HOOK(name, id, offset, Func)                                                                              \
    struct name                                                                                                        \
    {                                                                                                                  \
        static const inline auto Address = GetAddress(id, offset);                                                     \
        using FuncType                   = Func;                                                                       \
    }

namespace LIBC_NAMESPACE_DECL
{
    namespace SimpleIME
    {
        namespace Hooks
        {
            template <typename func_t>
            struct HookData
            {
            };

            template <typename return_t, typename... Args>
            struct HookData<return_t(Args...)>
            {
                using func_type = return_t(Args...);

                HookData(REL::RelocationID a_id, REL::VariantOffset a_offset, return_t (*funcPtr)(Args...))
                {
                    address = REL::Relocation<std::uint32_t>(a_id, a_offset).address();
                    trampoline.create(14);

                    auto ptr     = trampoline.write_call<5>(address, reinterpret_cast<void *>(funcPtr));
                    originalFunc = REL::Relocation<func_type>(ptr);
                }

                auto constexpr GetAddress() const -> std::uintptr_t
                {
                    return address;
                }

                auto operator()(Args... args) const noexcept
                {
                    originalFunc(args...);
                }

            private:
                SKSE::Trampoline                   trampoline{"CallHook"};
                std::uintptr_t                     address;
                REL::Relocation<return_t(Args...)> originalFunc;
            };

            struct D3DInitHookData : HookData<void()>
            {
                D3DInitHookData(func_type *ptr)
                    : HookData(REL::RelocationID(75595, 77226), REL::VariantOffset(0x9, 0x275, 0x00), ptr)
                {
                }
            };

            struct D3DPresentHookData : HookData<void(std::uint32_t)>
            {
                D3DPresentHookData(func_type *ptr)
                    : HookData(REL::RelocationID(75461, 77246), REL::VariantOffset(0x9, 0x9, 0x00), ptr)
                {
                }
            };

            struct DispatchInputEventHookData
                : HookData<void(RE::BSTEventSource<RE::InputEvent *> *, RE::InputEvent **)>
            {
                DispatchInputEventHookData(func_type *ptr)
                    : HookData(REL::RelocationID(67315, 68617), REL::VariantOffset(0x7B, 0x7B, 0x00), ptr)
                {
                }
            };

            template <typename func_t>
            static auto MakeHook(HookData<func_t> &a_hookData, func_t *funcPtr) -> REL::Relocation<func_t>
            {
                SKSE::Trampoline trampoline{"CallHook"};
                trampoline.create(14);

                auto ptr = trampoline.write_call<5>(a_hookData.GetAddress(), reinterpret_cast<void *>(funcPtr));
                a_hookData.SetOriginalFunc(ptr);
                return REL::Relocation<func_t>(ptr);
            }

            // Windows Hook
            using MYHOOKDATA = struct _MYHOOKDATA
            {
                int      nType;
                HOOKPROC hkprc;
                HHOOK    hhook;
            };

            LRESULT CALLBACK MyGetMsgProc(int code, WPARAM wParam, LPARAM lParam);
            void             InstallRegisterClassHook();
            void             InstallWindowsHooks();

            template <typename T>
            class CallHook
            {
            };

            template <typename R, typename... Args>
            class CallHook<R(Args...)>
            {
            public:
                CallHook(REL::RelocationID a_id, REL::VariantOffset offset, R (*funcPtr)(Args...))
                {
                    address = REL::Relocation<std::uintptr_t>(a_id, offset).address();

                    trampoline.create(14);
                    auto ptr     = trampoline.write_call<5>(address, reinterpret_cast<void *>(funcPtr));
                    originalFunc = ptr;
                }

                CallHook(std::uintptr_t a_address, R (*funcPtr)(Args...)) : address(a_address)
                {
                    trampoline.create(14);

                    auto ptr     = trampoline.write_call<5>(a_address, reinterpret_cast<void *>(funcPtr));
                    originalFunc = ptr;
                }

                auto operator()(Args... args) const noexcept -> R
                {
                    if constexpr (std::is_void_v<R>)
                    {
                        originalFunc(args...);
                    }
                    else
                    {
                        return originalFunc(args...);
                    }
                }

            private:
                SKSE::Trampoline            trampoline{"CallHook"};
                REL::Relocation<R(Args...)> originalFunc;
                std::uintptr_t              address;
            };
        };
    } // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL

#endif
