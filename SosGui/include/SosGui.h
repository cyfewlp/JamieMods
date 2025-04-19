#pragma once

#include "GuiContext.h"
#include "SosDataType.h"
#include "coroutine.h"
#include "data/SosUiData.h"
#include "data/id.h"
#include "gui/BaseGui.h"
#include "gui/OutfitListTable.h"
#include "gui/Table.h"
#include "service/OutfitService.h"
#include "service/SosDataCoordinator.h"

#include <RE/A/Actor.h>
#include <RE/R/Renderer.h>
#include <stdexcept>
#include <windows.h>

namespace LIBC_NAMESPACE_DECL
{

    class SosGui : public BaseGui
    {
        struct InitFail : std::runtime_error
        {
            explicit InitFail(const char *msg) : std::runtime_error(msg) {}
        };

        using Slot  = RE::BIPED_MODEL::BipedObjectSlot;
        using Armor = RE::TESObjectARMO;

        GuiContext         m_context;
        TableContext<2>    m_locationAutoSwitchTable;
        TableContext<3>    m_charactersTable;
        SosUiData          m_uiData;
        OutfitService      m_outfitService;
        SosDataCoordinator m_dataCoordinator;
        OutfitListTable    m_outfitListTable;

        bool m_fShowNearNpc = false;

    public:
        SosGui()
            : m_locationAutoSwitchTable(TableContext<2>::Create(
                  "##AutoSwitchStateList", {"$SosGui_TableHeader_Location", "$SosGui_TableHeader_Location_State"})),
              m_charactersTable(TableContext<3>::Create(
                  "##CharactersTable", {"$Characters", "$Delete", "$SosGui_TableHeader_ActiveOutfit"})),
              m_outfitService(m_uiData), m_dataCoordinator(m_uiData, m_outfitService),
              m_outfitListTable(m_uiData, m_outfitService)
        {
            m_locationAutoSwitchTable.Resizable().NoHostExtendX();
            m_charactersTable.Resizable().SizingStretchProp();
        }

        static auto Init(const RE::BSGraphics::RendererData &renderData, HWND hWnd) -> bool;

        auto Render() -> void;

        virtual auto Refresh() -> void override;

        CoroutinePromise operator<<(CoroutineTask &&task);

        virtual auto Close() -> void override
        {
            m_context.editingActor = nullptr;
            m_context.editingState = StateType::None;
            m_fShowNearNpc         = false;
            m_outfitListTable.Close();
        }

    private:
        auto DoRefresh() -> CoroutinePromise;

        auto        DoRender() -> void;
        static auto ThemeCombo() -> void;

        void ShowErrorMessages();

        void RenderQuickSlotConfig();

        void RenderExportOrImportSettings();

        void RefreshCurrentActorArmor();

        static void NewFrame();

        void RenderCharactersPanel();

        void RenderCharactersList();

        void CharactersContextMenu(const OutfitId &outfitId);

        void RenderNearNpcList();

        void RenderLocationBasedAutoswitch(RE::Actor *currentActor, ImVec2 &childSize);

        void ComboStateOutfitList(const StateType &state);

        static void TrySetAllowTextInput();

        static void AllowTextInput(bool allow);

        static void AllowTextInput1(RE::ControlMap *controlMap, bool allow);

        static auto EnableQuickslot(bool enable) -> bool;
    };
}