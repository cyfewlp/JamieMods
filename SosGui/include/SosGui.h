#pragma once

#include "SimpleIME/include/ImGuiThemeLoader.h"
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

        using Slot  = RE::BIPED_MODEL::BipedObjectSlot;
        using Armor = RE::TESObjectARMO;
        ImGuiUtil::ImTable<1> m_outfitListTable;
        ImGuiUtil::ImTable<2> m_locationAutoSwitchTable;
        ImGuiUtil::ImTable<2> m_charactersTable;

        bool        m_fShowMessage = false;
        std::string m_errorMessage;

        void InitTables();

        void AddErrorMessage(const char *msg)
        {
            m_errorMessage.assign(msg);
            m_fShowMessage = true;
        }

        void ShowErrorMessage();

    public:
        SosGui()
        {
            InitTables();
        }

        static auto Init(const RE::BSGraphics::RendererData &renderData, HWND hWnd) -> bool;

        auto        Render() -> void;
        static auto Refresh() -> void;

    private:
        auto DoRender() -> void;
        void RenderOutfitConfiguration();

        static void NewFrame();

        static void RenderQuickSlotConfig();
        void        RenderCharactersConfig();
        void        RenderCharactersList();
        void        RenderLocationBasedAutoswitch(RE::Actor *currentActor);
        static void TrySetAllowTextInput();
        static void AllowTextInput(bool allow);
        static void AllowTextInput1(RE::ControlMap *controlMap, bool allow);

        static auto EnableQuickslot(bool enable) -> bool;
        static auto HasQuickslotSpell() -> bool;
    };
}