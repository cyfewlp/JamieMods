#pragma once

#include "SosDataType.h"
#include "SosUiData.h"

#include <RE/R/Renderer.h>
#include <stdexcept>
#include <windows.h>

namespace LIBC_NAMESPACE_DECL
{
    class SosGui
    {
        struct InitFail : std::runtime_error
        {
            explicit InitFail(const char *msg) : std::runtime_error(msg)
            {
            }
        };

        bool                 m_fShow               = false;
        static constexpr int OUTFIT_NAME_MAX_BYTES = 256;
        static constexpr int MAX_FILTER_ARMOR_NAME = 256;

        using Slot = RE::BIPED_MODEL::BipedObjectSlot;

    public:
        static auto GetInstance() -> SosGui &
        {
            static SosGui g_instance;
            return g_instance;
        }

        static auto Init(const RE::BSGraphics::RendererData &renderData, HWND hWnd) -> bool;

        auto Render() -> void;
        auto ToggleShow() -> void;

    private:
        static void NewFrame();

        auto DoRender() -> void;
        void RenderCharactersConfig();
        void RenderCharactersList();
        void RenderLocationBasedAutoswitch(RE::Actor *currentActor);
        void RenderOutfitConfiguration();
        void RenderOutfitArmors(const std::string_view &outfitName);
        auto RenderArmorConflictPopup();

        static void RenderOutfitEditPanel(const std::string_view &outfitName);
        static void RenderOutfitAddPolicyById(const std::string_view &outfitName, bool &fFilterPlayable);
        static void RenderOutfitAddPolicyByCandidates(const std::string_view &outfitName);
        static auto RenderArmorSlotFilter() -> Slot;
        static void UpdateArmorCandidates(const std::string_view &filterString, bool mustBePlayable,
                                          OutfitAddPolicy policy);

        static void UpdateArmorCandidatesForAny(const std::string_view &filterString, bool mustBePlayable);

        static void FilterArmorCandidates(const std::string_view           &filterString,
                                          std::vector<RE::TESObjectARMO *> &armorCandidates);
        static auto IsFilterArmor(const std::string_view &filterString, RE::TESObjectARMO *armor) -> bool;
    };
}