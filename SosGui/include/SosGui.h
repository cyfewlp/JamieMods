#pragma once

#include "GuiContext.h"
#include "data/SosUiData.h"
#include "data/id.h"
#include "gui/BaseGui.h"
#include "gui/OutfitListTable.h"
#include "gui/Table.h"
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
class SosGui : public BaseGui
{
    struct InitFail : std::runtime_error
    {
        explicit InitFail(const char *msg) : std::runtime_error(msg) {}
    };

    struct outfit_debounce_input final : ImGuiUtil::debounce_input
    {
        std::vector<const SosUiOutfit *> viewData;

        outfit_debounce_input() : debounce_input("##filter", "filter outfit") {}

        void clear();
        void onInput() override;
        void updateView(const OutfitList &outfitList);
    };

    using Slot = RE::BIPED_MODEL::BipedObjectSlot;
    using Armor = RE::TESObjectARMO;

    GuiContext m_context;
    SosUiData m_uiData;
    OutfitService m_outfitService;
    SosDataCoordinator m_dataCoordinator;
    OutfitListTable m_outfitListTable;
    outfit_debounce_input m_outfitDebounceInput;

    bool m_fShowConfigWindows = true;

public:
    SosGui()
        : m_outfitService(m_uiData), m_dataCoordinator(m_uiData, m_outfitService),
          m_outfitListTable(m_uiData, m_outfitService) {}

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

    void NearNpcCombo() const;

    void AutoSwitchPoliesTable(RE::Actor *currentActor);

    void autoSwitch_column1_outfit(RE::FormID actorId, uint32_t policyId);

    // draw outfit select popup
    // return select outfit id or INVALID_OUTFIT_ID if not select.
    auto outfit_select_popup(__out ImGuiID &popupId) -> boost::optional<OutfitId>;

    static void TrySetAllowTextInput();

    static void AllowTextInput(bool allow);

    static void AllowTextInput1(RE::ControlMap *controlMap, bool allow);

    static auto EnableQuickslot(bool enable) -> bool;
};
}