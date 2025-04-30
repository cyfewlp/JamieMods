#pragma once

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
#include <stdexcept>
#include <windows.h>

namespace
LIBC_NAMESPACE_DECL
{
class SosGui final : public BaseGui
{
    struct InitFail final : std::runtime_error
    {
        explicit InitFail(const char *msg) : std::runtime_error(msg) {}
    };

    struct OutfitDebounceInput final : ImGuiUtil::DebounceInput
    {
        std::vector<const SosUiOutfit *> viewData{};

        void clear() override;
        void OnInput() override;
        void updateView(const OutfitList &outfitList);
    };

    struct outfit_select_popup : Popup::BasicPopup
    {
        OutfitDebounceInput &debounceInput;

        explicit outfit_select_popup(OutfitDebounceInput &debounceInput) : debounceInput(debounceInput) {}

        void draw(const char *nameKey, const OutfitList &outfitList, __out OutfitId &selectId);
    };

    struct autoSwitch_outfit_select_popup : outfit_select_popup
    {
        uint32_t selectPolicyId = -1;

        explicit autoSwitch_outfit_select_popup(OutfitDebounceInput &debounceInput)
            : outfit_select_popup(debounceInput) {}
    };

    using Slot = RE::BIPED_MODEL::BipedObjectSlot;
    using Armor = RE::TESObjectARMO;

    SosUiData m_uiData;
    OutfitService m_outfitService;
    SosDataCoordinator m_dataCoordinator;
    OutfitListTable m_outfitListTable;

    bool m_fShowConfigWindows = true;
    bool m_fWantTextInput = true; // previous frame WantTextInput state
    int m_selectedActorIndex = 0;
    RE::Actor*m_selectedActor = nullptr;

    OutfitDebounceInput m_outfitDebounceInput;
    outfit_select_popup m_outfitSelectPopup{m_outfitDebounceInput};
    autoSwitch_outfit_select_popup m_autoSwitchOutfitSelectPopup{m_outfitDebounceInput};

public:
    SosGui()
        : m_outfitService(m_uiData), m_dataCoordinator(m_uiData, m_outfitService),
          m_outfitListTable(m_uiData, m_outfitService) {}

    static auto Init(const RE::BSGraphics::RendererData &renderData, HWND hWnd) -> bool;

    auto Render() -> void;

    auto Refresh() -> void override;

    auto Close() -> void override;

private:
    auto DoRefresh() -> EagerTask;

    auto DoRender() -> void;
    void ToolbarWindow();
    void MainConfigWindow();

    auto GetSelectedActor() -> RE::Actor *
    {
        if (const auto &actors = m_uiData.GetActors(); static_cast<size_t>(m_selectedActorIndex) < actors.size())
        {
            return m_uiData.GetActors().at(m_selectedActorIndex);
        }
        return nullptr;
    }

    static auto ThemeCombo() -> void;

    void ShowErrorMessages();

    void RenderQuickSlotConfig();

    void RenderExportOrImportSettings();

    static void NewFrame();

    void RenderCharactersPanel();

    void RenderCharactersList();

    void AutoSwitchPoliesTable(RE::Actor *currentActor);

    void autoSwitch_column1_outfit(RE::FormID actorId, uint32_t policyId);

    void TrySetAllowTextInput();

    static void AllowTextInput(bool allow);

    static void AllowTextInput1(RE::ControlMap *controlMap, bool allow);

    static auto EnableQuickslot(bool enable) -> bool;
};
}