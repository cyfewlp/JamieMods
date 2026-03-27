#pragma once

#include "autoswitch/ActorPolicyView.h"
#include "data/SosUiData.h"
#include "gui/BaseGui.h"
#include "gui/CharacterEditPanel.h"
#include "gui/OutfitListTable.h"
#include "gui/popup/OutfitSelectPopup.h"
#include "i18n/Translator.h"
#include "i18n/translator_manager.h"
#include "service/OutfitService.h"
#include "service/SosDataCoordinator.h"
#include "task.h"

#include <RE/A/Actor.h>
#include <RE/R/Renderer.h>
#include <stdexcept>
#include <windows.h>

namespace SosGui
{
class SosGuiWindow
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
    CharacterEditPanel m_characterEditPanel;
    OutfitEditPanel    m_outfitEditPanel;
    OutfitListTable    m_outfitListTable;
    Context            m_context;
    i18n::Translator   m_translator;

    bool m_isShowPanels = true;

    AutoSwitch::ActorPolicyView        m_autoSwitchOutfitView{};
    std::unique_ptr<OutfitSelectPopup> m_outfitSelectPopup = nullptr;

public:
    SosGuiWindow();

    ~SosGuiWindow();

    static auto Init(HWND hWnd, const RE::BSGraphics::RendererData &renderData) -> void;
    static auto ShutDown() -> void;

    void Show()
    {
        m_characterEditPanel.Show();
        m_outfitListTable.Show();
        m_outfitEditPanel.Show();
    }

    auto Refresh() const -> EagerTask;
    auto OnPostDisplay() -> void;

private:
    void DrawTopModalPopup();
    auto Draw() -> void;
    auto DrawSidebar() -> float;
    void DockSpace();
    void Toolbar();

    auto GetSelectedActor(int index) -> RE::Actor *
    {
        if (const auto &actors = m_uiData.GetActors(); static_cast<size_t>(index) < actors.size())
        {
            return m_uiData.GetActors().at(index);
        }
        return nullptr;
    }

    void OnImportSettings();

    static auto EnableQuickslot(bool enable) -> bool;
};
} // namespace SosGui
