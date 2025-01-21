#pragma once

#include <windows.h>

namespace SimpleIME
{
    void             HookGetMsgProc();
    LRESULT CALLBACK MyGetMsgProc(int code, WPARAM wParam, LPARAM lParam);

    struct D3DInitHook
    {
        static constexpr REL::RelocationID  id{75595, 77226};
        static constexpr REL::VariantOffset offset{0x9, 0x275, 0x00};
        std::uintptr_t                      address;
        D3DInitHook()
        {
            REL::Relocation<std::uintptr_t> target{D3DInitHook::id, D3DInitHook::offset};
            address = target.address();
        }
    };

    struct D3DPresentHook
    {
        static constexpr auto id     = REL::RelocationID(75461, 77246);
        static constexpr auto offset = REL::VariantOffset(0x9, 0x9, 0x00);
        std::uintptr_t                      address;
        D3DPresentHook()
        {
            REL::Relocation<std::uintptr_t> target{D3DPresentHook::id, D3DPresentHook::offset};
            address = target.address();
        }
    };
} // namespace SimpleIME
