#pragma once

#include <d3d11.h>

#include <detours/detours.h>

namespace Hooks
{

    using namespace RE;
    void Install();

    struct InitWornFormHook
    {
        static constexpr auto id     = REL::ID(24736);
        static constexpr auto offset = REL::Offset(0x302);

        static void           Install();
        static void           InitWornForm(TESObjectARMO *a_armor, TESRace *a_race, BSTSmartPointer<BipedAnim> *a_biped,
                                           bool isFemale);
        using InitWornFormFunc = decltype(&InitWornForm);
        static inline REL::Relocation<InitWornFormFunc> originalFunc;
    };

    struct D3DInitHook
    {
        using D3DInitFunc            = void();

        static constexpr auto id     = REL::RelocationID(75595, 77226);
        static constexpr auto offset = REL::VariantOffset(0x9, 0x275, 0x00); // VR unknown
        static std::uintptr_t Install(D3DInitFunc *func);
    };

    struct D3DPresentHook
    {
        using D3DPresentFunc         = void __fastcall(std::uint32_t);
        static constexpr auto id     = REL::RelocationID(75461, 77246);
        static constexpr auto offset = REL::VariantOffset(0x9, 0x9, 0x00);
        static std::uintptr_t Install(D3DPresentFunc *func);
    };

    class DispatchInputEventHook
    {
    public:
        using RealFunc = void(BSTEventSource<InputEvent *> *, InputEvent **);
        static std::uintptr_t Install(RealFunc *func);

    private:
        static constexpr auto id     = REL::RelocationID(67315, 68617);
        static constexpr auto offset = REL::VariantOffset(0x7B, 0x7B, 0x00);
    };

    class GFXLoaderHook
    {
    public:
        static void Install();

    private:
        static constexpr auto id = Offset::GFxLoader::CreateMovie;
        using CreateMovieFunc    = GFxMovieDef *(GFxLoader *, char *, GFxLoader::LoadConstants, std::size_t);
        static inline REL::Relocation<CreateMovieFunc> originalFunc;
        static GFxMovieDef *MyCreateMovie(GFxLoader *, char *a_filename, GFxLoader::LoadConstants a_loadConstants,
                                          std::size_t a_memoryArena);
    };
} // namespace Hooks