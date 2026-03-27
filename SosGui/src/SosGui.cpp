#include "SosGui.h"

#include "WCharUtils.h"
#include "fonts/FontManager.h"
#include "gui/UiSettings.h"
#include "gui/icon.h"
#include "i18n/translator_manager.h"
#include "imgui.h"
#include "imgui/ImThemeLoader.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "imguiex/ImGuiEx.h"
#include "imguiex/imgui_manager.h"
#include "imguiex/imguiex_enum_wrap.h"
#include "imguiex/imguiex_m3.h"
#include "log.h"
#include "path_utils.h"
#include "task.h"
#include "util/ImGuiUtil.h"
#include "util/UiSettingsLoader.h"

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

SosGuiWindow::SosGuiWindow()
    : m_outfitService(m_uiData), m_dataCoordinator(m_uiData, m_outfitService), m_outfitEditPanel(m_uiData, m_outfitService),
      m_outfitListTable(m_uiData, m_outfitService, m_outfitEditPanel)
{
    i18n::SetTranslator(&m_translator);
    const auto modInterfaceDir = utils::GetInterfacePath() / SKSE::PluginDeclaration::GetSingleton()->GetName();
    i18n::UpdateTranslator("english", "english", modInterfaceDir);
}

SosGuiWindow::~SosGuiWindow()
{
    m_characterEditPanel.Cleanup();
    m_outfitListTable.Cleanup();
    m_outfitEditPanel.Cleanup();
    m_isShowPanels = true;
    i18n::SetTranslator(nullptr);
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

    ImGuiEx::M3::Initialize(utils::GetInterfaceFile(Settings::UiSettings::ICON_FONT), ImGuiEx::M3::GetM3ClassicSchemeConfig());
    ImGui::StyleColorsDark();
}

auto SosGuiWindow::ShutDown() -> void
{
    auto *uiSetting = Settings::UiSettings::GetInstance();
    Settings::Save(*uiSetting);
    ImGuiEx::M3::Destroy();
    ImGuiEx::Shutdown();
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

auto SosGuiWindow::OnPostDisplay() -> void
{
    ImGuiEx::NewFrame();

    m_uiData.ExecuteUiTasks();
    Draw();

    ImGuiEx::Render();
    ImGuiEx::EndFrame();
}

void SosGuiWindow::OnImportSettings()
{
    m_characterEditPanel.OnRefresh();
    m_outfitListTable.OnRefresh();
    m_outfitEditPanel.OnRefresh();
}

auto SosGuiWindow::Draw() -> void
{
    DockSpace();
    ErrorNotifier::GetInstance().Show();

    if (!m_isShowPanels)
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
        auto FocusWindowButton = [](std::string_view icon, const char *tooltip, BaseGui &baseGui) {
            auto isClick = ImGuiUtil::IconButton(icon);
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

        FocusWindowButton(ICON_USERS, Translate1("CharacterEditPanel"), m_characterEditPanel);
        FocusWindowButton(ICON_SHIRT, Translate1("Outfit"), m_outfitListTable);
        FocusWindowButton(ICON_FILE_PLUS_CORNER, Translate1("EditOutfit"), m_outfitEditPanel);
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
    if (ImGui::BeginMenu(Translate1("ToolBar.File")))
    {
        {
            bool fEnabled = m_uiData.IsEnabled();
            if (ImGui::Checkbox(Translate1("Enabled"), &fEnabled))
            {
                +[&] {
                    return m_dataCoordinator.RequestEnable(fEnabled);
                };
            }
        }

        {
            bool quickSlotEnabled = m_uiData.IsQuickSlotEnabled();
            if (ImGui::Checkbox(Translate1("ToolBar.Quickslots"), &quickSlotEnabled))
            {
                if (EnableQuickslot(quickSlotEnabled))
                {
                    m_uiData.SetQuickSlotEnabled(quickSlotEnabled);
                }
            }
            ImGui::SetItemTooltip("%s", Translate1("ToolBar.QuickslotsToolTip"));
        }

        if (ImGui::MenuItem(Translate1("ToolBar.Import")))
        {
            OnImportSettings();
            +[&] {
                return m_dataCoordinator.RequestImportSettings();
            };
        }

        if (ImGui::MenuItem(Translate1("ToolBar.Export")))
        {
            +[&] {
                return m_dataCoordinator.RequestExportSettings();
            };
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem(Translate1("ToolBar.ShowOrHide")))
    {
        m_isShowPanels = !m_isShowPanels;
    }
    if (ImGui::MenuItem(Translate1("ToolBar.RefreshPlayerArmor")))
    {
        util::RefreshActorArmor(RE::PlayerCharacter::GetSingleton());
    }
    if (ImGui::MenuItem(Translate1("ToolBar.Settings")))
    {
        ImGuiUtil::OpenPopup(Translate1("ToolBar.Settings"));
    }
    if (ImGui::MenuItem(Translate1("ToolBar.Close")))
    {
        auto *messageQueue = RE::UIMessageQueue::GetSingleton();
        messageQueue->AddMessage("SosGuiMenu", RE::UI_MESSAGE_TYPE::kHide, nullptr);
    }
    if (ImGui::MenuItem(Translate1("About")))
    {
        ImGuiUtil::OpenPopup("A Extra GUI for SkyrimOutfitSystemRE");
    }

    Popup::DrawSettingsPopup(Translate("ToolBar.Settings"));
    Popup::DrawAboutPopup("A Extra GUI for SkyrimOutfitSystemRE");

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
