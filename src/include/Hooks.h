#pragma once

#include "Ext.h"
#include <d3d11.h>

namespace Hooks {

    void Install();

    struct InitWornFormHook {
        static constexpr auto id = REL::ID(24736);
        static constexpr auto offset = REL::Offset(0x302);

        static void Install();
        static void InitWornForm(RE::TESObjectARMO* a_armor, RE::TESRace* a_race,
            RE::BSTSmartPointer<RE::BipedAnim>* a_biped, bool isFemale);
        using InitWornFormFunc = decltype(&InitWornForm);
        static inline REL::Relocation<InitWornFormFunc> originalFunc;
    };

    struct WndProcHook {
        static LRESULT thunk(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        static inline WNDPROC func;
    };

    struct D3DInitHook {
        static void thunk();
        static inline REL::Relocation<decltype(&thunk)> func;

        static constexpr auto id = REL::RelocationID(75595, 77226);
        static constexpr auto offset = REL::VariantOffset(0x9, 0x275, 0x00); // VR unknown

        static inline std::atomic<bool> initialized = false;
        static void Install();
    };

    struct D3DPresentHook {
        static void __fastcall thunk(std::uint32_t);
        static constexpr auto id = REL::RelocationID(75461, 77246);
        static constexpr auto offset = REL::VariantOffset(0x9, 0x9, 0x00);
        static inline REL::Relocation<decltype(thunk)> func;
        static void Install();
    };

} // namespace Hooks