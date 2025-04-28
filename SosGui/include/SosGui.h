#pragma once

#include "GuiContext.h"
#include "data/AutoSwitchPolicyView.h"
#include "data/SosUiData.h"
#include "data/id.h"
#include "gui/BaseGui.h"
#include "gui/OutfitListTable.h"
#include "gui/Popup.h"
#include "service/OutfitService.h"
#include "service/SosDataCoordinator.h"
#include "task.h"

#include <RE/A/Actor.h>
#include <RE/R/Renderer.h>
#include <cstdint>
#include <optional>
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

    struct outfit_debounce_input final : ImGuiUtil::debounce_input
    {
        std::vector<const SosUiOutfit *> viewData{};

        outfit_debounce_input() : debounce_input("##filter", "filter outfit") {}

        void clear();
        void onInput() override;
        void updateView(const OutfitList &outfitList);
    };

    struct outfit_select_popup : Popup::PopupContext
    {
        outfit_debounce_input &debounceInput;

        explicit outfit_select_popup(outfit_debounce_input &debounceInput) : debounceInput(debounceInput) {}

        virtual auto preDraw() -> bool;
        void         draw(const OutfitList &outfitList, __out OutfitId &selectId);
    };

    struct autoSwitch_outfit_select_popup : outfit_select_popup
    {
        uint32_t selectPolicyId = -1;

        explicit autoSwitch_outfit_select_popup(outfit_debounce_input &debounceInput)
            : outfit_select_popup(debounceInput)
        {}

        auto preDraw() -> bool override;
    };

    using Slot  = RE::BIPED_MODEL::BipedObjectSlot;
    using Armor = RE::TESObjectARMO;

    GuiContext         m_context;
    SosUiData          m_uiData;
    OutfitService      m_outfitService;
    SosDataCoordinator m_dataCoordinator;
    OutfitListTable    m_outfitListTable;

    bool m_fShowConfigWindows = true;
    bool m_fWantTextInput     = true; // previous frame WantTextInput state
    int  m_selectedActorIndex = 0;
    int  m_selectedNpcIndex   = 0;

    outfit_debounce_input          m_outfitDebounceInput;
    outfit_select_popup            m_outfitSelectPopup{m_outfitDebounceInput};
    autoSwitch_outfit_select_popup m_autoSwitchOutfitSelectPopup{m_outfitDebounceInput};

public:
    SosGui()
        : m_outfitService(m_uiData), m_dataCoordinator(m_uiData, m_outfitService),
          m_outfitListTable(m_uiData, m_outfitService)
    {}

    static auto Init(const RE::BSGraphics::RendererData &renderData, HWND hWnd) -> bool;

    auto Render() -> void;

    auto Refresh() -> void override;

    auto Close() -> void override
    {
        m_context.editingActor = nullptr;
        m_outfitListTable.Close();
    }

private:
    auto DoRefresh() -> EagerTask;

    auto DoRender() -> void;
    void ToolbarWindow();
    void MainConfigWindow();

    static auto ThemeCombo() -> void;

    void ShowErrorMessages();

    void RenderQuickSlotConfig();

    void RenderExportOrImportSettings();

    void RefreshCurrentActorArmor() const;

    static void NewFrame();

    void RenderCharactersPanel();

    void RenderCharactersList();

    void NearNpcCombo();

    void AutoSwitchPoliesTable(RE::Actor *currentActor);

    void autoSwitch_column1_outfit(RE::FormID actorId, uint32_t policyId);

    void TrySetAllowTextInput();

    static void AllowTextInput(bool allow);

    static void AllowTextInput1(RE::ControlMap *controlMap, bool allow);

    static auto EnableQuickslot(bool enable) -> bool;
};
}