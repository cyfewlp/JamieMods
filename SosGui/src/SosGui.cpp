#include "SosGui.h"

#include "common/config.h"
#include "common/imgui/ImGuiFlags.h"
#include "common/imgui/ImGuiScope.h"
#include "common/imgui/ImThemeLoader.h"
#include "common/log.h"
#include "gui/Table.h"
#include "gui/UiSetting.h"
#include "gui/icon.h"
#include "gui/popup/SettingsPopup.h"
#include "imgui.h"
#include "imgui_freetype.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "task.h"
#include "util/ImGuiSettinsHandler.h"
#include "util/ImGuiUtil.h"

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

inline void SosGui::OutfitDebounceInput::Clear()
{
    DebounceInput::Clear();
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
    static std::string IniFileName;

    constexpr auto mainFontName = R"(C:\Windows\Fonts\simsun.ttc)";
    const auto     iconFontName = util::GetInterfaceFile(Setting::UiSetting::ICON_FONT);
    constexpr auto MonaspaceXenonFont =
        R"(D:\assets\monaspace-v1.200\monaspace-v1.200\fonts\frozen\MonaspaceXenonFrozen-Regular.ttf)";
    constexpr auto emojiFont = R"(C:\Windows\Fonts\seguiemj.ttf)";

    ImFontConfig fontConfig;
    ImFontConfig mergeableConfig;
    mergeableConfig.MergeMode = true;

    fontConfig.OversampleH = fontConfig.OversampleV = 1;
    fontConfig.GlyphExcludeRanges                   = io.Fonts->GetGlyphRangesDefault();
    fontConfig.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;

    io.Fonts->AddFontFromFileTTF(emojiFont, Setting::UiSetting::FONT_SIZE_TEXT, &fontConfig);
    io.Fonts->AddFontFromFileTTF(MonaspaceXenonFont, Setting::UiSetting::FONT_SIZE_TEXT, &mergeableConfig);
    io.Fonts->AddFontFromFileTTF(mainFontName, Setting::UiSetting::FONT_SIZE_TEXT, &mergeableConfig);
    auto *iconFont =
        io.Fonts->AddFontFromFileTTF(iconFontName.c_str(), Setting::UiSetting::FONT_SIZE_TEXT, &mergeableConfig);
    io.Fonts->Build();
    IniFileName    = util::GetInterfaceFile(io.IniFilename);
    io.IniFilename = IniFileName.c_str();

    Context::GetInstance().SetIconFont(iconFont);

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

void SosGui::OnRefresh()
{
    m_outfitListTable.OnRefresh();
    m_outfitEditPanel.OnRefresh();
    m_selectedActorIndex = 0;
}

auto SosGui::Cleanup() -> void
{
    m_outfitListTable.Cleanup();
    m_outfitEditPanel.Cleanup();
    m_fShowConfigWindows = true;
    m_selectedActorIndex = 0;
}

void SosGui::DrawTopModalPopup()
{
    auto &context = Context::GetInstance();
    if (context.popupList.empty())
    {
        return;
    }

    ImGui::PushStyleVarX(ImGuiStyleVar_WindowPadding, 25.0F);
    ImGuiScope::FontSize fontSize4(Setting::UiSetting::FONT_SIZE_TITLE_4);
    const auto          &modalPopup = context.popupList.front();
    bool                 confirmed  = false;
    const bool           toErase    = !modalPopup->Draw(m_uiData, confirmed, ImGuiWindowFlags_AlwaysAutoResize);
    if (confirmed && !m_outfitListTable.OnModalPopupConfirmed(modalPopup.get()))
    {
        m_outfitEditPanel.OnModalPopupConfirmed(modalPopup.get());
    }
    if (toErase)
    {
        context.popupList.pop_front();
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
        auto      &context       = Context::GetInstance();
        m_outfitListTable.Draw(context, selectedActor);

        const auto &editingOutfit = m_outfitListTable.GetEditingOutfit();
        m_outfitEditPanel.Draw(context, editingOutfit);
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

    const float offsetY = viewport->WorkSize.y * 0.25;

    float width = 0.0;
    if (ImGui::Begin(
            "##MainSidebar", nullptr, ImGuiUtil::WindowFlags().AlwaysAutoResize().NoDecoration().NoDocking().NoMove()
        ))
    {
        width = ImGui::GetWindowWidth();
        ImGui::SetCursorPosY(offsetY);
        auto           framePadding = ImGuiScope::StyleVar::FramePadding(Setting::UiSetting::ICON_PADDING);
        auto           buttonColor  = ImGuiScope::StyleColor::Button(ImVec4(0, 0, 0, 0));
        auto           fontSize     = ImGuiScope::FontSize(Setting::UiSetting::FONT_SIZE_TITLE_3);
        constexpr auto IconButton   = [](const char *iconClass, const char *tooltip) {
            ImGui::PushFont(Context::GetInstance().GetIconFont());
            auto isClick = ImGui::Button(iconClass);
            ImGui::PopFont();
            ImGui::SetItemTooltip("%s", tooltip);
            return isClick;
        };
        if (IconButton(NF_FA_SHIRT, "$SosGui_Outfit"_T.c_str()))
        {
            m_outfitListTable.ToggleShow();
        }

        ImGui::Dummy(ImVec2{1, fontSize * 0.5f});
        if (IconButton(NF_OCT_GEAR, "$SkyOutSys_MCM_Options"_T.c_str()))
        {
            ToggleShow();
        }

        ImGui::Dummy(ImVec2{1, fontSize * 0.5f});
        if (IconButton(NF_FA_EDIT, "$SosGui_EditOutfit"_T.c_str()))
        {
            m_outfitEditPanel.ToggleShow();
        }
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
    ImGui::PushStyleVarX(ImGuiStyleVar_ItemSpacing, 15);
    ImGuiScope::StyleVar::FramePadding({3, 3});
    if (ImGui::BeginMenu(std::format("{} {}", NF_OCT_FILE, "$SosGui_ToolBar_File"_T.c_str()).c_str()))
    {
        if (ImGui::MenuItem("$SkyOutSys_Text_Import"_T.c_str()))
        {
            OnRefresh();
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
    if (ImGuiUtil::MenuItem("$SosGui_ToolBar_ShowOrHide"))
    {
        m_fShowConfigWindows = !m_fShowConfigWindows;
    }
    if (ImGuiUtil::MenuItem("$SosGui_ToolBar_RefreshPlayerArmor"))
    {
        util::RefreshActorArmor(RE::PlayerCharacter::GetSingleton());
    }
    if (ImGui::MenuItem(std::format("{} {}", NF_OCT_GEAR, "$SosGui_ToolBar_Settings"_T).c_str()))
    {
        Context::GetInstance().popupList.push_back(std::make_unique<Popup::SettingsPopup>("$SosGui_ToolBar_Settings"));
    }
    if (ImGui::MenuItem(std::format("{} {}", NF_MD_CLOSE, "$SosGui_ToolBar_Close"_T.c_str()).c_str()))
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
        DrawQuickSlotConfig();
        DrawCharactersPanel();

        const RE::Actor *selectedActor = GetSelectedActor();
        m_autoSwitchOutfitView.Draw(selectedActor, m_uiData, m_dataCoordinator, m_outfitService);
    }
    ImGui::End();
}

void SosGui::DrawQuickSlotConfig()
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