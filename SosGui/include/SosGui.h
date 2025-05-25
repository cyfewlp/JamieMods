#pragma once

#include "autoswitch/ActorPolicyView.h"
#include "data/SosUiData.h"
#include "gui/BaseGui.h"
#include "gui/OutfitListTable.h"
#include "gui/popup/OutfitSelectPopup.h"
#include "service/OutfitService.h"
#include "service/SosDataCoordinator.h"
#include "task.h"

#include <RE/A/Actor.h>
#include <RE/R/Renderer.h>
#include <stdexcept>
#include <windows.h>

namespace LIBC_NAMESPACE_DECL
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

        void Clear() override;
        void OnInput() override;
        void updateView(const OutfitList &outfitList);
    };

    using Slot  = RE::BIPED_MODEL::BipedObjectSlot;
    using Armor = RE::TESObjectARMO;

    SosUiData          m_uiData;
    OutfitService      m_outfitService;
    SosDataCoordinator m_dataCoordinator;
    OutfitEditPanel    m_outfitEditPanel;
    OutfitListTable    m_outfitListTable;

    bool       m_fShowConfigWindows = true;
    bool       m_fWantTextInput     = true; // previous frame WantTextInput state
    int        m_selectedActorIndex = 0;
    RE::Actor *m_selectedActor      = nullptr;

    AutoSwitch::ActorPolicyView        m_autoSwitchOutfitView{};
    std::unique_ptr<OutfitSelectPopup> m_outfitSelectPopup = nullptr;

public:
    SosGui()
        : m_outfitService(m_uiData), m_dataCoordinator(m_uiData, m_outfitService),
          m_outfitEditPanel(m_uiData, m_outfitService), m_outfitListTable(m_uiData, m_outfitService, m_outfitEditPanel)
    {
    }

    static auto Init(const RE::BSGraphics::RendererData &renderData, HWND hWnd) -> bool;
    static auto ShutDown() -> void;

    void Show() override
    {
        BaseGui::Show();
        m_outfitListTable.Show();
        m_outfitEditPanel.Show();
    }

    void Focus() override;
    void OnRefresh() override;
    void Cleanup() override;

    auto Refresh() const -> EagerTask;
    auto Render() -> void;

private:
    void DrawTopModalPopup();
    auto DoRender() -> void;
    auto DrawSidebar() -> float;
    void DockSpace();
    void Toolbar();
    void MainConfigWindow();

    auto GetSelectedActor() -> RE::Actor *
    {
        if (const auto &actors = m_uiData.GetActors(); static_cast<size_t>(m_selectedActorIndex) < actors.size())
        {
            return m_uiData.GetActors().at(m_selectedActorIndex);
        }
        return nullptr;
    }

    static void NewFrame();

    void DrawCharactersPanel();

    void DrawCharactersList();

    void autoSwitch_column1_outfit(RE::FormID actorId, uint32_t policyId);

    void TrySetAllowTextInput();

    static void AllowTextInput(bool allow);

    static void AllowTextInput1(RE::ControlMap *controlMap, bool allow);

    static auto EnableQuickslot(bool enable) -> bool;
};
}