#ifndef HOOKS_HPP
#define HOOKS_HPP

#pragma once

#include "configs/Configs.h"
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

            using FuncRegisterClass      = ATOM (*)(const WNDCLASSA *);
            using FuncDirectInput8Create = HRESULT WINAPI (*)(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN);
            static inline FuncRegisterClass        RealRegisterClassExA   = nullptr;
            static inline FuncDirectInput8Create   RealDirectInput8Create = nullptr;

            // Windows Hook
            using MYHOOKDATA = struct _MYHOOKDATA
            {
                int      nType;
                HOOKPROC hkprc;
                HHOOK    hhook;
            };

            LRESULT CALLBACK MyGetMsgProc(int code, WPARAM wParam, LPARAM lParam);
            void             InstallRegisterClassHook();
            void             InstallDirectInPutHook();
            void             InstallWindowsHooks();

            auto WINAPI      MyRegisterClassExA(const WNDCLASSA *wndClass) -> ATOM;
            auto WINAPI      MyDirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut,
                                                  LPUNKNOWN punkOuter) -> HRESULT;

        };
    } // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL

#endif
