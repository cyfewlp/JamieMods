#include "SosGui.h"

#include "WCharUtils.h"
#include "fonts/FontManager.h"
#include "gui/UiSettings.h"
#include "gui/icon.h"
#include "gui/popup/AboutPopup.h"
#include "gui/popup/SettingsPopup.h"
#include "imgui.h"
#include "imgui/ImThemeLoader.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "imguiex/ImGuiEx.h"
#include "imguiex/imgui_manager.h"
#include "imguiex/imguiex_enum_wrap.h"
#include "log.h"
#include "task.h"
#include "util/ImGuiUtil.h"
#include "util/UiSettingsLoader.h"

#include <RE/C/ControlMap.h>
#include <RE/C/CursorMenu.h>
#include <RE/M/MenuCursor.h>
#include <RE/P/PlayerCharacter.h>
#include <RE/R/Renderer.h>
#include <RE/S/SpellItem.h>
#include <RE/T/TESForm.h>
#include <RE/U/UI.h>
#include <REL/Relocation.h>
#include <SKSE/Impl/PCH.h>
#include <windows.h>

namespace SosGui
{
inline void SosGuiWindow::OutfitDebounceInput::Clear()
{
    DebounceInput::Clear();
    viewData.clear();
}

void SosGuiWindow::OutfitDebounceInput::OnInput()
{
    DebounceInput::OnInput();
    viewData.clear();
}

void SosGuiWindow::OutfitDebounceInput::updateView(const OutfitList &outfitList)
{
    dirty = false;
    viewData.clear();
    outfitList.for_each([&](const auto &outfit, int64_t) {
        if (filter.PassFilter(outfit.GetName().c_str()))
        {
            viewData.push_back(&outfit);
        }
    });
}

auto SosGuiWindow::Init(const HWND hWnd, const RE::BSGraphics::RendererData &renderData) -> void
{
    auto *uiSetting = ::SosGui::Settings::UiSettings::GetInstance();
    Settings::Load(*uiSetting);

    auto *device  = reinterpret_cast<ID3D11Device *>(renderData.forwarder);
    auto *context = reinterpret_cast<ID3D11DeviceContext *>(renderData.context);
    ImGuiEx::Initialize(hWnd, device, context);
    if (const auto defaultFontFilePath = Fonts::GetDefaultFontFilePath(); !defaultFontFilePath.empty())
    {
        (void)ImGuiEx::AddPrimaryFont({WCharUtils::ToString(defaultFontFilePath)}, {});
    }

    ImGui::StyleColorsDark();
}

auto SosGuiWindow::ShutDown() -> void
{
    auto *uiSetting = Settings::UiSettings::GetInstance();
    Settings::Save(*uiSetting);
    ImGuiEx::Shutdown();
}

auto SosGuiWindow::Cleanup() -> void
{
    m_characterEditPanel.Cleanup();
    m_outfitListTable.Cleanup();
    m_outfitEditPanel.Cleanup();
    m_fShowPanels = true;
}

void SosGuiWindow::DrawTopModalPopup()
{
    if (m_context.popupList.empty())
    {
        return;
    }

    ImGui::PushStyleVarX(ImGuiStyleVar_WindowPadding, 25.0F);

    ImGui::PushFont(nullptr, Settings::UiSettings::GetInstance()->Title4PxSize());
    const auto &modalPopup = m_context.popupList.front();
    bool        confirmed  = false;
    const bool  toErase    = !modalPopup->Draw(m_uiData, confirmed, ImGuiWindowFlags_AlwaysAutoResize);
    if (confirmed && !m_outfitListTable.OnModalPopupConfirmed(modalPopup.get()))
    {
        m_outfitEditPanel.OnModalPopupConfirmed(modalPopup.get());
    }
    if (toErase)
    {
        m_context.popupList.pop_front();
    }
    ImGui::PopStyleVar();
    ImGui::PopFont();
}

auto SosGuiWindow::Refresh() const -> EagerTask
{
    co_await m_dataCoordinator.Refresh();
}

auto SosGuiWindow::Render() -> void
{
    ImGuiEx::NewFrame();

    m_uiData.ExecuteUiTasks();
    DrawPanels();

    ImGuiEx::Render();
    ImGuiEx::EndFrame();
}

void SosGuiWindow::OnImportSettings()
{
    m_characterEditPanel.OnRefresh();
    m_outfitListTable.OnRefresh();
    m_outfitEditPanel.OnRefresh();
}

auto SosGuiWindow::DrawPanels() -> void
{
    DockSpace();
    ErrorNotifier::GetInstance().Show();

    if (!m_fShowPanels)
    {
        return;
    }
    try
    {
        m_characterEditPanel.Draw(m_uiData, m_dataCoordinator, m_outfitService);

        DrawTopModalPopup();

        const auto &selectActorIndex = m_characterEditPanel.GetSelectedActorIndex();
        RE::Actor  *selectedActor    = GetSelectedActor(selectActorIndex);
        m_outfitListTable.Draw(m_context, selectedActor);

        const auto &editingOutfit = m_outfitListTable.GetEditingOutfit();
        m_outfitEditPanel.Draw(m_context, editingOutfit);
    }
    catch (const std::exception &e)
    {
        logger::LogStacktrace();
        ErrorNotifier::GetInstance().Error(e.what());
    }
}

auto SosGuiWindow::DrawSidebar() -> float
{
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos({0, viewport->WorkPos.y});
    ImGui::SetNextWindowSize({0, viewport->WorkSize.y});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {5.0F, 10.0F});

    const float offsetY = viewport->WorkSize.y * 0.25F;

    float width = 0.0;
    if (ImGui::Begin("##MainSidebar", nullptr, ImGuiEx::WindowFlags().AlwaysAutoResize().NoDecoration().NoMove()))
    {
        width = ImGui::GetWindowWidth();
        ImGui::SetCursorPosY(offsetY);
        const auto styleGuard = ImGuiEx::StyleGuard().Style<ImGuiStyleVar_FramePadding>({5.0F, 5.0F}).Color<ImGuiCol_Button>({});
        ImGui::PushFont(nullptr, Settings::UiSettings::GetInstance()->Title3PxSize());
        auto FocusWindowButton = [](const char *iconClass, const char *tooltip, BaseGui &baseGui) {
            auto isClick = ImGui::Button(iconClass);
            ImGui::SetItemTooltip("%s", tooltip);
            if (isClick)
            {
                if (baseGui.IsFocused())
                {
                    baseGui.ToggleShow();
                }
                else
                {
                    baseGui.Focus();
                }
            }
        };

        FocusWindowButton(NF_OCT_PEOPLE, "$SosGui_CharacterEditPanel"_T.c_str(), m_characterEditPanel);
        FocusWindowButton(NF_FA_SHIRT, "$SosGui_Outfit"_T.c_str(), m_outfitListTable);
        FocusWindowButton(NF_FA_EDIT, "$SosGui_EditOutfit"_T.c_str(), m_outfitEditPanel);
        ImGui::PopFont();
    }
    ImGui::End();
    ImGui::PopStyleVar();
    return width;
}

void SosGuiWindow::DockSpace()
{
    float sideBarWidth = DrawSidebar();

    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos({sideBarWidth, viewport->WorkPos.y});
    ImGui::SetNextWindowSize(viewport->WorkSize);
    // ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    constexpr auto windowFlags =
        ImGuiEx::WindowFlags().MenuBar().NoTitleBar().NoCollapse().NoResize().NoMove().NoBringToFrontOnFocus().NoNavFocus().NoBackground();
    ImGui::PushStyleVarY(ImGuiStyleVar_FramePadding, ImGui::GetFontSize() * 0.3f); // padding menu bar
    ImGui::Begin("DockSpace Demo", nullptr, windowFlags);
    ImGui::PopStyleVar(4);

    // Submit the DockSpace
    // const ImGuiID dockSpaceId = ImGui::GetID("MyDockSpace");
    // ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

    if (ImGui::BeginMenuBar())
    {
        Toolbar();
        ImGui::EndMenuBar();
    }

    ImGui::End();
}

void SosGuiWindow::Toolbar()
{
    ImGui::PushStyleVarX(ImGuiStyleVar_ItemSpacing, 15);
    const auto styleGuard = ImGuiEx::StyleGuard().Style<ImGuiStyleVar_FramePadding>({3.0F, 3.0F});
    if (ImGui::BeginMenu(std::format("{} {}", NF_OCT_FILE, "$SosGui_ToolBar_File"_T).c_str()))
    {
        {
            bool fEnabled = m_uiData.IsEnabled();
            if (ImGui::Checkbox("$Enabled"_T.c_str(), &fEnabled))
            {
                +[&] {
                    return m_dataCoordinator.RequestEnable(fEnabled);
                };
            }
        }

        {
            bool quickSlotEnabled = m_uiData.IsQuickSlotEnabled();
            if (ImGui::Checkbox("$SkyOutSys_MCMHeader_Quickslots"_T.c_str(), &quickSlotEnabled))
            {
                if (EnableQuickslot(quickSlotEnabled))
                {
                    m_uiData.SetQuickSlotEnabled(quickSlotEnabled);
                }
            }
            ImGui::SetItemTooltip("%s", "$SkyOutSys_Desc_EnableQuickslots"_T.c_str());
        }

        if (ImGui::MenuItem("$SkyOutSys_Text_Import"_T.c_str()))
        {
            OnImportSettings();
            +[&] {
                return m_dataCoordinator.RequestImportSettings();
            };
        }
        ImGui::SetItemTooltip("%s", "$SkyOutSys_Desc_Import"_T.c_str());

        if (ImGui::MenuItem("$SkyOutSys_Text_Export"_T.c_str()))
        {
            +[&] {
                return m_dataCoordinator.RequestExportSettings();
            };
        }
        ImGuiUtil::SetItemTooltip("$SkyOutSys_Text_Export");
        ImGui::EndMenu();
    }
    if (ImGuiUtil::MenuItem(std::format("{} {}", m_fShowPanels ? NF_OCT_EYE : NF_OCT_EYE_CLOSED, "$SosGui_ToolBar_ShowOrHide"_T).c_str()))
    {
        m_fShowPanels = !m_fShowPanels;
    }
    if (ImGuiUtil::MenuItem(std::format("{} {}", NF_MD_REFRESH, "$SosGui_ToolBar_RefreshPlayerArmor"_T).c_str()))
    {
        util::RefreshActorArmor(RE::PlayerCharacter::GetSingleton());
    }
    if (ImGui::MenuItem(std::format("{} {}", NF_OCT_GEAR, "$SosGui_ToolBar_Settings"_T).c_str()))
    {
        m_context.popupList.push_back(std::make_unique<Popup::SettingsPopup>("$SosGui_ToolBar_Settings"));
    }
    if (ImGui::MenuItem(std::format("{} {}", NF_MD_CLOSE, "$SosGui_ToolBar_Close"_T).c_str()))
    {
        auto *messageQueue = RE::UIMessageQueue::GetSingleton();
        messageQueue->AddMessage("SosGuiMenu", RE::UI_MESSAGE_TYPE::kHide, nullptr);
    }
    if (ImGui::MenuItem("$SosGui_About"_T.c_str()))
    {
        m_context.popupList.push_back(std::make_unique<Popup::AboutPopup>("$SosGui_About"));
    }
    ImGui::PopStyleVar();
}

EagerTask waitImport(const SosDataCoordinator &dataCoordinator)
{
    logger::debug("wait import");
    co_await dataCoordinator.RequestImportSettings();
}

auto SosGuiWindow::EnableQuickslot(const bool enable) -> bool
{
    const auto &player = RE::PlayerCharacter::GetSingleton();
    if (auto *spell = RE::TESForm::LookupByEditorID<RE::SpellItem>(SOS_SPELL_EDITOR_ID); spell != nullptr)
    {
        enable ? player->AddSpell(spell) : player->RemoveSpell(spell);
        return true;
    }
    return false;
}
} // namespace SosGui
