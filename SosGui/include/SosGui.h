#pragma once

#include "SosUiData.h"
#include "gui/SosDataCoordinator.h"

#include <RE/A/Actor.h>
#include <RE/R/Renderer.h>
#include <SosOutfit.h>
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
        ImGuiUtil::ImTable<1> m_outfitListTable;
        ImGuiUtil::ImTable<2> m_locationAutoSwitchTable;
        ImGuiUtil::ImTable<3> m_charactersTable;
        RE::Actor            *m_editingActor  = nullptr;
        SosOutfit            *m_editingOutfit = nullptr;
        SosUiData             m_uiData;
        SosDataCoordinator    m_dataCoordinator;

        void InitTables();

    public:
        SosGui() : m_dataCoordinator(m_uiData)
        {
            InitTables();
        }

        static auto Init(const RE::BSGraphics::RendererData &renderData, HWND hWnd) -> bool;

        auto Render() -> void;
        auto Refresh() -> void;

    private:
        auto DoRender() -> void;

        void RenderQuickSlotConfig();
        void RenderExportOrImportSettings();
        void RenderOutfitConfiguration();
        void RenderEditingOutfit();

        void RenderOutfitListContextMenu(const std::string &outfitName);
        void ContextMenuSetActorActiveOutfit(std::string outfitName);
        void ContextMenuDeleteOutfit(std::string outfitName);
        void RefreshCurrentActorArmor();
        void OnSelectOutfit(SosOutfit &outfit, bool prevSelectState);

        static void NewFrame();

        void        RenderCharactersPanel();
        void        RenderCharactersList();
        static void RenderNearNpcList();
        void        RenderLocationBasedAutoswitch(RE::Actor *currentActor);
        static void TrySetAllowTextInput();
        static void AllowTextInput(bool allow);
        static void AllowTextInput1(RE::ControlMap *controlMap, bool allow);

        static auto EnableQuickslot(bool enable) -> bool;
    };
}