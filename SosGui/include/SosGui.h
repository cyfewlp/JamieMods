#pragma once

#include "SimpleIME/include/ImGuiThemeLoader.h"
#include "SosDataType.h"
#include "SosUiData.h"

#include <RE/R/Renderer.h>
#include <stdexcept>
#include <windows.h>

namespace LIBC_NAMESPACE_DECL
{

    template <size_t Columns>
    struct ImTable
    {
        std::string                      name;
        uint32_t                         rows;
        ImGuiTableFlags                  flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders;
        std::array<std::string, Columns> headersRow;

        ImTable() : rows(0), flags(0)
        {
        }

        ImTable(std::string_view name, uint32_t rows, std::array<std::string_view, Columns> headersRow)
            : name(name), rows(rows), headersRow(headersRow)
        {
        }
    };

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

        using Slot  = RE::BIPED_MODEL::BipedObjectSlot;
        using Armor = RE::TESObjectARMO;
        ImTable<1> m_outfitListTable;
        ImTable<2> m_outfitArmorsTable;
        ImTable<3> m_armorCandidatesTable;
        ImTable<2> m_locationAutoSwitchTable;

        void InitTables();

    public:
        SosGui()
        {
            InitTables();
        }

        static auto Init(const RE::BSGraphics::RendererData &renderData, HWND hWnd) -> bool;

        auto        Render() -> void;
        static auto Refresh() -> void;
        static void RenderQuickslotConfig();

    private:
        auto DoRender() -> void;
        void RenderOutfitArmors(const std::string &outfitName);
        void RenderOutfitConfiguration();
        void RenderOutfitEditPanel(const std::string &outfitName);
        void RenderOutfitAddPolicyByCandidates(const std::string &outfitName);

        static void NewFrame();

        auto RenderArmorConflictPopup();

        void        RenderCharactersConfig();
        void        RenderCharactersList();
        void        RenderLocationBasedAutoswitch(RE::Actor *currentActor);
        static void RenderOutfitEditPanelPolicy(const std::string &outfitName);
        static void TrySetAllowTextInput();
        static void RenderOutfitAddPolicyById(const std::string &outfitName, const bool &fFilterPlayable);
        static auto RenderArmorSlotFilter() -> Slot;
        static void UpdateArmorCandidates(const std::string_view &filterString, bool mustBePlayable,
                                          OutfitAddPolicy policy);
        static void AllowTextInput(bool allow);
        static void AllowTextInput1(RE::ControlMap *controlMap, bool allow);

        static void UpdateArmorCandidatesForAny(const std::string_view &filterString, bool mustBePlayable);
        static void UpdateArmorCandidatesBySlot(Slot slot);

        static void FilterArmorCandidates(const std::string_view           &filterString,
                                          std::vector<RE::TESObjectARMO *> &armorCandidates);
        static auto IsFilterArmor(const std::string_view &filterString, RE::TESObjectARMO *armor) -> bool;

        static auto EnableQuickslot(bool enable) -> bool;
        static auto HasQuickslotSpell() -> bool;
    };
}