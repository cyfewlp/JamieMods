#pragma once

#include "ImGuiUtil.h"
#include "SosDataType.h"
#include "SosUiData.h"
#include "data/SosUiOutfit.h"
#include "gui/SosDataCoordinator.h"
#include "gui/SosGuiOutfit.h"
#include "gui/Popup.h"
#include "gui/Table.h"

#include <RE/A/Actor.h>
#include <RE/R/Renderer.h>
#include <stdexcept>
#include <string>
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

        using Slot  = RE::BIPED_MODEL::BipedObjectSlot;
        using Armor = RE::TESObjectARMO;
        TableContext<1> m_outfitListTable;
        TableContext<2> m_locationAutoSwitchTable;
        TableContext<3> m_charactersTable;

        RE::Actor               *m_editingActor           = nullptr;
        SosUiOutfit             *m_editingOutfit          = nullptr;
        SosUiOutfit             *m_selectedOutfit         = nullptr;
        StateType                m_editingAutoSwitchState = StateType::None;
        bool                     m_fShowNearNpc           = false;
        SosUiData                m_uiData;
        SosDataCoordinator       m_dataCoordinator;
        SosGuiOutfit             m_guiOutfit;
        Popup::DeleteOutfitPopup m_DeleteOutfitPopup;

    public:
        SosGui()
            : m_outfitListTable(TableContext<1>::Create("##OutfitLists", {"$SkyOutSys_MCM_OutfitList"})),
              m_locationAutoSwitchTable(TableContext<2>::Create(
                  "##AutoSwitchStateList", {"$SosGui_TableHeader_Location", "$SosGui_TableHeader_Location_State"})),
              m_charactersTable(TableContext<3>::Create(
                  "##CharactersTable", {"$Characters", "$Delete", "$SosGui_TableHeader_ActiveOutfit"})),
              m_dataCoordinator(m_uiData), m_guiOutfit(m_uiData, m_dataCoordinator)
        {
            m_outfitListTable.NoHostExtendX();
            m_locationAutoSwitchTable.Sortable().Resizable().NoHostExtendX();
            m_charactersTable.Sortable().Resizable().SizingStretchProp();
        }

        static auto Init(const RE::BSGraphics::RendererData &renderData, HWND hWnd) -> bool;

        auto Render() -> void;
        auto Refresh() -> void;

        auto Close() -> void
        {
            m_editingActor           = nullptr;
            m_editingOutfit          = nullptr;
            m_editingAutoSwitchState = StateType::None;
            m_fShowNearNpc           = false;
        }

    private:
        auto DoRender() -> void;
        void ShowErrorMessages();

        void RenderQuickSlotConfig();
        void RenderExportOrImportSettings();
        void RenderOutfitConfiguration(const ImVec2 &childSize);
        void RenderEditingOutfit();
        void RenderPopups();

        void RefreshCurrentActorArmor();

        void RenderOutfitListContextMenu(SosUiOutfit &outfit);
        void ContextMenuSetActorActiveOutfit(std::string outfitName);
        void OnAcceptEditingOutfit(SosUiOutfit &outfit);
        void OnAcceptOutfitForState(const std::string &outfitName);

        static void NewFrame();

        void RenderCharactersPanel();
        void RenderCharactersList();
        void RenderNearNpcList();
        void RenderLocationBasedAutoswitch(RE::Actor *currentActor, ImVec2 &childSize);
        void ComboStateOutfitList(const StateType &state);

        static void TrySetAllowTextInput();
        static void AllowTextInput(bool allow);
        static void AllowTextInput1(RE::ControlMap *controlMap, bool allow);

        static auto EnableQuickslot(bool enable) -> bool;
    };
}