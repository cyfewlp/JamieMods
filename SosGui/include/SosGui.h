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

        static auto Render() -> void;
        static auto Refresh() -> void;
        static void RenderQuickslotConfig();

    private:
        static void NewFrame();

        auto RenderArmorConflictPopup();

        static auto DoRender() -> void;
        static void RenderOutfitConfiguration();
        static void RenderCharactersConfig();
        static void RenderCharactersList();
        static void RenderOutfitArmors(const std::string_view &outfitName);
        static void TrySetAllowTextInput();
        static void RenderLocationBasedAutoswitch(RE::Actor *currentActor);
        static void RenderOutfitEditPanel(const std::string_view &outfitName);
        static void RenderOutfitAddPolicyById(const std::string_view &outfitName, const bool &fFilterPlayable);
        static void RenderOutfitAddPolicyByCandidates(const std::string_view &outfitName);
        static auto RenderArmorSlotFilter() -> Slot;
        static void UpdateArmorCandidates(const std::string_view &filterString, bool mustBePlayable,
                                          OutfitAddPolicy policy);
        static void AllowTextInput(bool allow);
        static void AllowTextInput1(RE::ControlMap *controlMap, bool allow);

        static void UpdateArmorCandidatesForAny(const std::string_view &filterString, bool mustBePlayable);

        static void FilterArmorCandidates(const std::string_view           &filterString,
                                          std::vector<RE::TESObjectARMO *> &armorCandidates);
        static auto IsFilterArmor(const std::string_view &filterString, RE::TESObjectARMO *armor) -> bool;

        static auto EnableQuickslot(bool enable) -> bool;
        static auto HasQuickslotSpell() -> bool;
    };
}