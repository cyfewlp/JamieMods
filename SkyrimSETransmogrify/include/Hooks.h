#pragma once

#include <d3d11.h>

namespace Hooks {

    void Install();

    struct InitWornFormHook
    {
        static constexpr auto id = REL::ID(24736);
        static constexpr auto offset = REL::Offset(0x302);

        static void Install();
        static void InitWornForm(RE::TESObjectARMO* a_armor,
                                 RE::TESRace* a_race,
                                 RE::BSTSmartPointer<RE::BipedAnim>* a_biped,
                                 bool isFemale);
        using InitWornFormFunc = decltype(&InitWornForm);
        static inline REL::Relocation<InitWornFormFunc> originalFunc;
    };

    struct D3DInitHook
    {
        using D3DInitFunc = void();

        static constexpr auto id = REL::RelocationID(75595, 77226);
        static constexpr auto offset = REL::VariantOffset(0x9, 0x275, 0x00); // VR unknown
        static std::uintptr_t Install(D3DInitFunc* func);
    };

    struct D3DPresentHook
    {
        using D3DPresentFunc = void __fastcall(std::uint32_t);
        static constexpr auto id = REL::RelocationID(75461, 77246);
        static constexpr auto offset = REL::VariantOffset(0x9, 0x9, 0x00);
        static std::uintptr_t Install(D3DPresentFunc* func);
    };

    class DispatchInputEventHook
    {
    public:
        using RealFunc = void(RE::BSTEventSource<RE::InputEvent*>*, RE::InputEvent**);
        static std::uintptr_t Install(RealFunc* func);

    private:
        static constexpr auto id = REL::RelocationID(67315, 68617);
        static constexpr auto offset = REL::VariantOffset(0x7B, 0x7B, 0x00);
    };

} // namespace Hooks