#include "SosGui.h"

#include "common/config.h"
#include "common/imgui/ImGuiScop.h"
#include "common/imgui/ImGuiFlags.h"
#include "common/log.h"
#include "gui/Config.h"
#include "imgui.h"
#include "imgui_freetype.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "task.h"
#include "util/ImGuiSettinsHandler.h"
#include "util/ImGuiUtil.h"
#include "util/ImThemeLoader.h"

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

namespace LIBC_NAMESPACE_DECL
{

inline void SosGui::OutfitDebounceInput::clear()
{
    DebounceInput::clear();
    viewData.clear();
}

void SosGui::OutfitDebounceInput::OnInput()
{
    DebounceInput::OnInput();
    viewData.clear();
}

void SosGui::OutfitDebounceInput::updateView(const OutfitList &outfitList)
{
    dirty = false;
    viewData.clear();
    outfitList.for_each([&](const auto &outfit, size_t) {
        if (filter.PassFilter(outfit.GetName().c_str()))
        {
            viewData.push_back(&outfit);
        }
    });
}

auto SosGui::Init(const RE::BSGraphics::RendererData &renderData, HWND hWnd) -> bool
{
    auto *device  = reinterpret_cast<ID3D11Device *>(renderData.forwarder);
    auto *context = reinterpret_cast<ID3D11DeviceContext *>(renderData.context);

    log_info("Initializing ImGui...");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    if (!ImGui_ImplWin32_Init(hWnd))
    {
        throw InitFail("ImGui initialization failed (Win32)");
    }

    if (!ImGui_ImplDX11_Init(device, context))
    {
        throw InitFail("ImGui initialization failed (DX11)");
    }

    RECT     rect = {0, 0, 0, 0};
    ImGuiIO &io   = ImGui::GetIO();
    io.ConfigFlags |= ImGuiUtil::ConfigFlags().NavEnableKeyboard().DockingEnable();
    io.ConfigNavMoveSetMousePos = false;
    ::GetClientRect(hWnd, &rect);
    io.DisplaySize = ImVec2(static_cast<float>(rect.right - rect.left), static_cast<float>(rect.bottom - rect.top));

    ImGui::StyleColorsDark();
    constexpr auto mainFont = R"(C:\Windows\Fonts\simsun.ttc)";
    constexpr auto MonaspaceXenonFont =
        R"(D:\assets\monaspace-v1.200\monaspace-v1.200\fonts\frozen\MonaspaceXenonFrozen-Regular.ttf)";
    constexpr auto emojiFont = R"(C:\Windows\Fonts\seguiemj.ttf)";

    ImFontConfig fontConfig;
    fontConfig.OversampleH = fontConfig.OversampleV = 1;
    fontConfig.GlyphExcludeRanges                   = io.Fonts->GetGlyphRangesDefault();
    fontConfig.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;

    const auto &pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();

    io.Fonts->AddFontFromFileTTF(emojiFont, 14.0F, &fontConfig);
    ImFontConfig fontConfig1;
    fontConfig1.MergeMode = true;
    io.Fonts->AddFontFromFileTTF(MonaspaceXenonFont, 14.0F, &fontConfig1);
    io.Fonts->AddFontFromFileTTF(mainFont, 14.0F, &fontConfig1);
    io.Fonts->Build();
    static std::string IniFileName;
    IniFileName    = std::format(Config::IMGUI_INI_FILE_TEMPLATE, pluginName.data());
    io.IniFilename = IniFileName.c_str();

    ImGuiStyle &style = ImGui::GetStyle();
    if ((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0)
    {
        style.WindowRounding              = 0.0F;
        style.Colors[ImGuiCol_WindowBg].w = 1.0F;
    }

    // add our setting handler
    ImGuiSettingsHandler ini_handler;
    ini_handler.TypeName   = SKSE::PluginDeclaration::GetSingleton()->GetName().data();
    ini_handler.TypeHash   = ImHashStr(ini_handler.TypeName);
    ini_handler.ReadOpenFn = Setting::UiSettingReadOpenFn;
    ini_handler.ReadLineFn = Setting::UiSettingReadLineFn;
    ini_handler.WriteAllFn = Setting::UiSettingWriteAll;
    ini_handler.ClearAllFn = Setting::UiSettingClearAll;
    ini_handler.ApplyAllFn = Setting::UiSettingApplyAll;
    ImGui::AddSettingsHandler(&ini_handler);

    log_info("ImGui initialized!");
    return true;
}

auto SosGui::Cleanup() -> void
{
    m_outfitListTable.Cleanup();
    m_outfitEditPanel.Cleanup();
    m_outfitDebounceInput.clear();
    m_fShowConfigWindows                         = true;
    m_selectedActorIndex                         = 0;
    m_autoSwitchOutfitSelectPopup.selectPolicyId = -1;
    m_outfitDebounceInput.clear();
}

void SosGui::DrawTopModalPopup()
{
    if (m_context.popupList.empty())
    {
        return;
    }

    ImGui::PushStyleVarX(ImGuiStyleVar_WindowPadding, 25.0F);
    ImGuiScope::FontSize fontSize4(Config::FONT_SIZE_TITLE_4);
    const auto          &modalPopup = m_context.popupList.front();
    bool                 confirmed  = false;
    const bool           toErase    = !modalPopup->Draw(m_uiData, confirmed, ImGuiWindowFlags_AlwaysAutoResize);
    if (confirmed && !m_outfitListTable.OnModalPopupConfirmed(modalPopup.get()))
    {
        m_outfitEditPanel.OnModalPopupConfirmed(modalPopup.get());
    }
    if (toErase)
    {
        m_context.popupList.pop_front();
    }
    ImGui::PopStyleVar();
}

auto SosGui::Refresh() const -> EagerTask
{
    co_await m_dataCoordinator.Refresh();
}

void SosGui::NewFrame()
{
    if (auto *ui = RE::UI::GetSingleton(); ui != nullptr)
    {
        POINT cursorPos;
        if (ui->IsMenuOpen(RE::CursorMenu::MENU_NAME))
        {
            const auto *menuCursor = RE::MenuCursor::GetSingleton();
            ImGui::GetIO().AddMouseSourceEvent(ImGuiMouseSource_Mouse);
            ImGui::GetIO().AddMousePosEvent(menuCursor->cursorPosX, menuCursor->cursorPosY);
        }
        else if (GetCursorPos(&cursorPos) != FALSE)
        {
            ImGui::GetIO().AddMousePosEvent(static_cast<float>(cursorPos.x), static_cast<float>(cursorPos.y));
        }
    }
}

auto SosGui::Render() -> void
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    NewFrame();
    ImGui::NewFrame();

    TrySetAllowTextInput();

    m_uiData.ExecuteUiTasks();
    DoRender();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    if ((ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void SosGui::TrySetAllowTextInput()
{
    const bool cWantTextInput = ImGui::GetIO().WantTextInput;
    if (!m_fWantTextInput && cWantTextInput)
    {
        AllowTextInput(true);
    }
    else if (m_fWantTextInput && !cWantTextInput)
    {
        AllowTextInput(false);
    }

    m_fWantTextInput = cWantTextInput;
}

auto SosGui::DoRender() -> void
{
    DockSpace();
    m_uiData.GetErrorNotifier().show();

    if (!m_fShowConfigWindows)
    {
        return;
    }
    try
    {
        if (IsShowing())
        {
            MainConfigWindow();
        }

        DrawTopModalPopup();

        RE::Actor *selectedActor = GetSelectedActor();
        m_outfitListTable.Draw(m_context, selectedActor);

        const auto &editingOutfit = m_outfitListTable.GetEditingOutfit();
        m_outfitEditPanel.Draw(m_context, editingOutfit);
    }
    catch (const std::exception &e)
    {
        LogStacktrace();
        m_uiData.PushErrorMessage(e.what());
    }
}

auto SosGui::DrawSidebar() -> float
{
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos({0, viewport->WorkPos.y});
    ImGui::SetNextWindowSize({0, viewport->WorkSize.y});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {5.0f, 10.0f});

    const float &fontSize = ImGui::GetFontSize();
    const float  offsetY  = viewport->WorkSize.y * 0.25;

    float width = 0.0;
    if (ImGui::Begin(
            "##MainSidebar", nullptr, ImGuiUtil::WindowFlags().AlwaysAutoResize().NoDecoration().NoDocking().NoMove()
        ))
    {
        width = ImGui::GetWindowWidth();
        ImGui::SetCursorPosY(offsetY);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        if (ImGui::Button("🥼"))
        {
            m_outfitListTable.ToggleShow();
        }
        ImGui::SetItemTooltip("$SosGui_Outfit");

        ImGui::Dummy(ImVec2{1, fontSize * 0.5f});
        if (ImGui::Button("⚙"))
        {
            ToggleShow();
        }
        ImGui::SetItemTooltip("$SkyOutSys_MCM_Options");

        ImGui::Dummy(ImVec2{1, fontSize * 0.5f});
        if (ImGui::Button("📝"))
        {
            m_outfitEditPanel.ToggleShow();
        }
        ImGui::SetItemTooltip("$SosGui_EditOutfit");
        ImGui::PopStyleColor();
    }
    ImGui::End();
    ImGui::PopStyleVar();
    return width;
}

void SosGui::DockSpace()
{
    float sideBarWidth = DrawSidebar();

    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos({sideBarWidth, viewport->WorkPos.y});
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    auto windowFlags = ImGuiUtil::WindowFlags().MenuBar().NoDocking().NoTitleBar().NoCollapse().NoResize().NoMove();
    windowFlags |= ImGuiUtil::WindowFlags().NoBringToFrontOnFocus().NoNavFocus().NoBackground();
    ImGui::Begin("DockSpace Demo", nullptr, windowFlags);
    ImGui::PopStyleVar(3);

    // Submit the DockSpace
    ImGuiIO &io = ImGui::GetIO();
    assert(io.ConfigFlags & ImGuiConfigFlags_DockingEnable && "Must enable docking");
    const ImGuiID dockSpaceId = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

    if (auto menuBar = ImGuiScope::MenuBar())
    {
        Toolbar();
    }

    ImGui::End();
}

void SosGui::Toolbar()
{
    ImGui::PushStyleVarX(ImGuiStyleVar_ItemSpacing, 20);
    if (ImGuiUtil::MenuItem("$SosGui_ToolBar_ShowOrHide"))
    {
        m_fShowConfigWindows = !m_fShowConfigWindows;
    }
    if (ImGuiUtil::MenuItem("$SosGui_ToolBar_RefreshPlayerArmor"))
    {
        util::RefreshActorArmor(RE::PlayerCharacter::GetSingleton());
    }
    if (ImGuiUtil::MenuItem("$SosGui_ToolBar_Close"))
    {
        auto *messageQueue = RE::UIMessageQueue::GetSingleton();
        messageQueue->AddMessage("SosGuiMenu", RE::UI_MESSAGE_TYPE::kHide, nullptr);
    }
    ImGui::PopStyleVar();
}

void SosGui::MainConfigWindow()
{
    ImGui::SetNextWindowPos(ImVec2(DEFAULT_MAIN_WINDOW_POS_X, DEFAULT_WINDOW_POS_Y), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("SosGuiOptions", &m_show, ImGuiWindowFlags_NoNav))
    {
        bool fEnabled = m_uiData.IsEnabled();
        if (ImGuiUtil::CheckBox("$Enabled", &fEnabled))
        {
            +[&] {
                return m_dataCoordinator.RequestEnable(fEnabled);
            };
        }
        ImGui::Indent(2);
        ImGui::SameLine();
        if (ImGui::DragFloat(
                "$SosGui_Global_FontSize_Scale"_T.c_str(),
                &ImGui::GetIO().FontGlobalScale,
                0.05F,
                Setting::UiSetting::FONT_SCALE_MIN,
                Setting::UiSetting::FONT_SCALE_MAX
            ))
        {
            Setting::UiSetting::GetInstance()->globalFontScale = ImGui::GetIO().FontGlobalScale;
        }
        ThemeCombo();

        RenderQuickSlotConfig();
        ImGui::SameLine();
        DrawExportOrImportSettings();

        RenderCharactersPanel();

        RE::Actor *selectedActor = GetSelectedActor();
        AutoSwitchPoliesTable(selectedActor);
    }
    ImGui::End();
}

auto SosGui::ThemeCombo() -> void
{
    auto         *settings    = Setting::UiSetting::GetInstance();
    const int32_t themeIndex  = settings->selectedThemeIndex;
    using Loader              = ImThemeLoader::Loader;
    const std::string preview = Loader::IsIndexInRange(themeIndex) ? Loader::g_availableThemes[themeIndex] : "";
    ImGuiScope::Combo combo("Theme", preview.c_str());
    if (!combo)
    {
        return;
    }
    size_t index = 0;
    for (const auto &themeName : Loader::g_availableThemes)
    {
        if (ImGui::Selectable(themeName.c_str(), false))
        {
            Loader::UseTheme(index);
            settings->selectedThemeIndex = index;
        }
        ++index;
    }
}

void SosGui::RenderQuickSlotConfig()
{
    bool quickSlotEnabled = m_uiData.IsQuickSlotEnabled();
    if (ImGui::Checkbox("$SkyOutSys_MCMHeader_Quickslots"_T.c_str(), &quickSlotEnabled))
    {
        if (!EnableQuickslot(quickSlotEnabled))
        {
            m_uiData.SetQuickSlotEnabled(false);
        }
    }

    ImGui::SetItemTooltip("%s", "$SkyOutSys_Desc_EnableQuickslots"_T.c_str());
}

EagerTask waitImport(const SosDataCoordinator &dataCoordinator)
{
    log_debug("wait import");
    co_await dataCoordinator.RequestImportSettings();
}

void SosGui::DrawExportOrImportSettings()
{
    if (ImGui::Button("$SkyOutSys_Text_Export"_T.c_str()))
    {
        +[&] {
            return m_dataCoordinator.RequestExportSettings();
        };
    }
    ImGuiUtil::SetItemTooltip("$SkyOutSys_Text_Export");

    ImGui::SameLine();
    if (ImGui::Button("$SkyOutSys_Text_Import"_T.c_str()))
    {
        Cleanup();
        +[&] {
            return m_dataCoordinator.RequestImportSettings();
        };
    }
    ImGui::SetItemTooltip("%s", "$SkyOutSys_Desc_Import"_T.c_str());
}

auto SosGui::EnableQuickslot(const bool enable) -> bool
{
    const auto &player = RE::PlayerCharacter::GetSingleton();
    if (auto *spell = RE::TESForm::LookupByEditorID<RE::SpellItem>(SOS_SPELL_EDITOR_ID); spell != nullptr)
    {
        enable ? player->AddSpell(spell) : player->RemoveSpell(spell);
        return true;
    }
    return false;
}

void SosGui::AllowTextInput(const bool allow)
{
    AllowTextInput1(RE::ControlMap::GetSingleton(), allow);
}

void SosGui::AllowTextInput1(RE::ControlMap *controlMap, bool allow)
{
    using func_t = decltype(&SosGui::AllowTextInput1);
    static REL::Relocation<func_t> func{RELOCATION_ID(67252, 68552)};
    func(controlMap, allow);
}

}