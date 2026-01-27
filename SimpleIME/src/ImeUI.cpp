//
// Created by jamie on 25-1-21.
//

#define IMGUI_DEFINE_MATH_OPERATORS

#include "ImeUI.h"

#include "ImeWnd.hpp"
#include "common/WCharUtils.h"
#include "common/config.h"
#include "common/imgui/ErrorNotifier.h"
#include "common/imgui/ImGuiEx.h"
#include "common/imgui/Material3.h"
#include "common/imgui/ThemesLoader.h"
#include "common/imgui/imgui_m3_ex.h"
#include "common/log.h"
#include "configs/CustomMessage.h"
#include "core/State.h"
#include "icons.h"
#include "ime/ImeController.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "menu/MenuNames.h"
#include "tsf/LangProfileUtil.h"
#include "ui/Settings.h"
#include "ui/TaskQueue.h"
#include "utils/FocusGFxCharacterInfo.h"

#include <RE/M/MenuCursor.h>
#include <RE/U/UIMessage.h>
#include <RE/U/UIMessageQueue.h>
#include <SKSE/Interfaces.h>
#include <WinUser.h>
#include <cguid.h>
#include <climits>
#include <cstdint>
#include <expected>
#include <format>
#include <math.h>
#include <minwindef.h>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <winnt.h>

#pragma comment(lib, "dwrite.lib")

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{

ImeUI::~ImeUI()
{
    if (m_pTextService != nullptr)
    {
        m_langProfileUtil->Release();
    }
}

bool ImeUI::Initialize(LangProfileUtil *pLangProfileUtil, const Settings &settings)
{
    log_debug("Initializing ImeUI...");
    m_langProfileUtil = pLangProfileUtil;
    m_langProfileUtil->AddRef();
    if (!m_langProfileUtil->LoadAllLangProfiles())
    {
        log_error("Failed load lang profiles");
        return false;
    }
    if (!m_langProfileUtil->LoadActiveIme())
    {
        log_error("Failed load active ime");
        return false;
    }
    if (!m_translation.GetTranslateLanguages(settings.resources.translationDir, m_translateLanguages))
    {
        log_warn("Failed load translation languages.");
    }
    m_fontBuilder.Initialize();
    return true;
}

// FIXME: should be move out to anther class
void ApplyM3Colors(const ImGuiEx::M3::Colors &colors)
{
    using namespace ImGuiEx::M3;
    ImGuiStyle &style = ImGui::GetStyle();

    style.Colors[ImGuiCol_WindowBg]      = colors.Surface();
    style.Colors[ImGuiCol_Text]          = colors.OnSurface();
    style.Colors[ImGuiCol_TitleBg]       = colors.SurfaceContainerHighest();
    style.Colors[ImGuiCol_TitleBgActive] = Colors::GetActiveColor(colors.SurfaceContainerHighest(), colors.OnSurface());
    style.Colors[ImGuiCol_TitleBgCollapsed]     = colors.SecondaryContainer();
    style.Colors[ImGuiCol_ChildBg]              = colors.Surface();
    style.Colors[ImGuiCol_PopupBg]              = colors.PrimaryContainer();
    style.Colors[ImGuiCol_FrameBg]              = colors.SecondaryContainer();
    style.Colors[ImGuiCol_FrameBgActive]        = colors.SecondaryContainer();
    style.Colors[ImGuiCol_FrameBgHovered]       = colors.SecondaryContainer();
    style.Colors[ImGuiCol_SliderGrab]           = colors.Primary();
    style.Colors[ImGuiCol_SliderGrabActive]     = Colors::GetActiveColor(colors.Primary(), colors.OnPrimary());
    style.Colors[ImGuiCol_Button]               = colors.Primary();
    style.Colors[ImGuiCol_ButtonHovered]        = Colors::GetHoveredColor(colors.Primary(), colors.OnPrimary());
    style.Colors[ImGuiCol_ButtonActive]         = Colors::GetActiveColor(colors.Primary(), colors.OnPrimary());
    style.Colors[ImGuiCol_ScrollbarBg]          = {0, 0, 0, 0};
    style.Colors[ImGuiCol_ScrollbarGrab]        = colors.Outline();
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = colors.OutlineVariant();
    style.Colors[ImGuiCol_ScrollbarGrabActive]  = colors.Primary();

    style.Colors[ImGuiCol_MenuBarBg] = colors.SurfaceContainerHigh();

    style.Colors[ImGuiCol_Header]        = colors.SurfaceContainerHigh();
    style.Colors[ImGuiCol_HeaderHovered] = Colors::GetHoveredColor(colors.SurfaceContainerHigh(), colors.OnSurface());
    style.Colors[ImGuiCol_HeaderActive]  = Colors::GetActiveColor(colors.SurfaceContainerHigh(), colors.OnSurface());

    style.Colors[ImGuiCol_Separator]        = colors.Secondary();
    style.Colors[ImGuiCol_SeparatorHovered] = Colors::GetHoveredColor(colors.Secondary(), colors.OnSecondary());
    style.Colors[ImGuiCol_SeparatorActive]  = Colors::GetActiveColor(colors.Secondary(), colors.OnSecondary());

    style.Colors[ImGuiCol_ResizeGrip] = colors.SecondaryContainer();
    style.Colors[ImGuiCol_ResizeGripHovered] =
        Colors::GetHoveredColor(colors.SecondaryContainer(), colors.OnSecondaryContainer());
    style.Colors[ImGuiCol_ResizeGripActive] =
        Colors::GetActiveColor(colors.SecondaryContainer(), colors.OnSecondaryContainer());

    style.Colors[ImGuiCol_InputTextCursor] = colors.Secondary();

    style.Colors[ImGuiCol_Tab]                 = colors.Surface();
    style.Colors[ImGuiCol_TabHovered]          = Colors::GetHoveredColor(colors.Surface(), colors.OnSurface());
    style.Colors[ImGuiCol_TabSelected]         = colors.Surface();
    style.Colors[ImGuiCol_TabSelectedOverline] = colors.Primary();

    style.Colors[ImGuiCol_TabDimmed]                 = colors.Surface();
    style.Colors[ImGuiCol_TabDimmedSelected]         = Colors::GetActiveColor(colors.Surface(), colors.OnSurface());
    style.Colors[ImGuiCol_TabDimmedSelectedOverline] = colors.OutlineVariant();

    style.Colors[ImGuiCol_PlotLines]        = colors.Primary();
    style.Colors[ImGuiCol_PlotLinesHovered] = Colors::GetHoveredColor(colors.OnPrimary(), colors.OnPrimary());

    style.Colors[ImGuiCol_PlotHistogram]        = colors.Tertiary();
    style.Colors[ImGuiCol_PlotHistogramHovered] = Colors::GetHoveredColor(colors.Tertiary(), colors.OnTertiary());

    style.Colors[ImGuiCol_TableHeaderBg]     = colors.SurfaceContainerHigh();
    style.Colors[ImGuiCol_TableBorderStrong] = colors.Outline();
    style.Colors[ImGuiCol_TableBorderLight]  = colors.OutlineVariant();
    style.Colors[ImGuiCol_TableRowBg]        = colors.Surface();
    style.Colors[ImGuiCol_TableRowBgAlt]     = colors.SurfaceContainerLowest();

    // style.Colors[ImGuiCol_TextLink]     =colors.surface_container_low;
    style.Colors[ImGuiCol_TextSelectedBg]   = colors.Primary();
    style.Colors[ImGuiCol_TextSelectedBg].w = 0.35f;

    style.Colors[ImGuiCol_TreeLines] = colors.OnSurface();

    style.Colors[ImGuiCol_DragDropTarget]   = colors.Primary();
    style.Colors[ImGuiCol_DragDropTargetBg] = colors.Surface();

    style.Colors[ImGuiCol_UnsavedMarker]         = colors.OnPrimary();
    style.Colors[ImGuiCol_NavCursor]             = colors.OnSecondary();
    style.Colors[ImGuiCol_NavWindowingHighlight] = colors.OnPrimary();
    style.Colors[ImGuiCol_NavWindowingDimBg]     = colors.SurfaceContainer();

    style.Colors[ImGuiCol_ModalWindowDimBg]   = colors.Surface();
    style.Colors[ImGuiCol_ModalWindowDimBg].w = 0.35f;
}

void ImeUI::ApplyAppearanceSettings(Settings &settings)
{
    auto &appearance = settings.appearance;

    if (const auto langIt = std::ranges::find(m_translateLanguages, appearance.language);
        langIt == m_translateLanguages.end())
    {
        appearance.language = "english";
    }
    m_translation.UseLanguage(appearance.language.c_str());

    ApplyM3Colors(m_styles.colors);
    // ApplyTheme(settings); // TODO: deprecated ImThemes???
}

void ImeUI::ApplyTheme(Settings &settings)
{
    m_themesLoader.LoadThemes();

    auto       &appearance = settings.appearance;
    const auto &themes     = m_themesLoader.GetThemes();

    ImGuiStyle                       style;
    std::expected<void, std::string> expected = std::unexpected("Theme not found");
    if (appearance.themeIndex < themes.size())
    {
        if (themes[appearance.themeIndex].name == appearance.theme)
        {
            expected = m_themesLoader.UseTheme(appearance.themeIndex, style);
        }
    }
    else
    {
        const auto findIt     = std::lower_bound(themes.begin(), themes.end(), ImGuiUtil::Theme(0, appearance.theme));
        appearance.themeIndex = static_cast<size_t>(std::distance(themes.begin(), findIt));
        if (appearance.themeIndex < themes.size())
        {
            expected = m_themesLoader.UseTheme(appearance.themeIndex, style);
        }
    }
    if (expected)
    {
        FillCommonStyleFields(style, settings);
        ImGui::GetStyle() = style;
    }
    else
    {
        ErrorNotifier::GetInstance().Warning(
            std::format("Can't find theme {}, fallback to ImGui default theme: {}", appearance.theme, expected.error())
        );
        if (m_themesLoader.UseTheme(0, ImGui::GetStyle()))
        {
            appearance.theme      = "Dark";
            appearance.themeIndex = 0;
        }
    }
    ImGuiUtil::ThemesLoader::Cleanup();
    ImGui::GetStyle().FontSizeBase = settings.state.fontSize;
}

void ImeUI::Draw(const Settings &settings)
{
    static bool shouldRelayout = true;
    static bool imeAppearing   = true;
    const auto &state          = State::GetInstance();
    const auto  id             = SKSE::PluginDeclaration::GetSingleton()->GetName();
    if (state.ImeDisabled() || !state.IsImeInputting() /* || !state.HasAny(State::KEYBOARD_OPEN, State::IME_OPEN)*/)
    {
        imeAppearing   = true;
        shouldRelayout = true;
        ImGui::CloseCurrentPopup();
        return;
    }
    if (imeAppearing)
    {
        imeAppearing = false;
        UpdateImeWindowPos(settings, m_imeWindowPos);
    }
    if (shouldRelayout)
    {
        shouldRelayout = false;
        ClampWindowToViewport(m_imeWindowSize, m_imeWindowPos);
        ImGui::SetNextWindowPos(m_imeWindowPos);
    }

    auto flags =
        ImGuiEx::WindowFlags().NoDecoration().AlwaysAutoResize().NoFocusOnAppearing().NoSavedSettings().NoNav();
    if (ImGui::Begin(id.data(), nullptr, flags))
    {
        ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
        if (state.Has(State::IN_COMPOSING))
        {
            DrawCompWindow(settings);
        }

        ImGui::Separator();
        // render ime status window: language,
        if (state.Has(State::IN_CAND_CHOOSING))
        {
            DrawCandidateWindows();
        }
        m_imeWindowSize = ImGui::GetWindowSize();
    }
    ImGui::End();
    if (IsImeNeedRelayout())
    {
        shouldRelayout = true;
    }
}

void ImeUI::DrawInputMethodsCombo() const
{
    auto &activatedGuid     = m_langProfileUtil->GetActivatedLangProfile();
    auto &installedProfiles = m_langProfileUtil->GetLangProfiles();
    if (!installedProfiles.contains(activatedGuid))
    {
        activatedGuid = GUID_NULL;
    }
    if (const auto &profile = installedProfiles[activatedGuid];
        ImGui::BeginCombo("###InstalledIME", profile.desc.c_str()))
    {
        int32_t idx = 0;
        for (const std::pair<GUID, LangProfile> pair : installedProfiles)
        {
            ImGui::PushID(idx);
            const auto &langProfile = pair.second;
            const bool  isSelected  = langProfile.guidProfile == activatedGuid;
            if (ImGui::Selectable(langProfile.desc.c_str()))
            {
                m_pImeWnd->SendMessageToIme(CM_ACTIVATE_PROFILE, 0, reinterpret_cast<LPARAM>(&pair.first));
                activatedGuid = m_langProfileUtil->GetActivatedLangProfile();
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
            ImGui::PopID();
            idx++;
        }
        ImGui::EndCombo();
    }
}

void ImeUI::ShowToolWindow()
{
    if (m_fPinToolWindow)
    {
        m_fPinToolWindow = false;
        // ImGui::GetIO().MouseDrawCursor = true;
        ImGui::SetWindowFocus(TOOL_WINDOW_NAME.data());
    }
    else
    {
        m_fShowToolWindow = !m_fShowToolWindow;
        // ImGui::GetIO().MouseDrawCursor = m_fShowToolWindow;
        if (m_fShowToolWindow)
        {
            ImGui::SetWindowFocus(TOOL_WINDOW_NAME.data());
        }
    }
}

void ImeUI::DrawToolWindow(Settings &settings)
{
    if (!m_fShowToolWindow)
    {
        return;
    }

    m_translation.UseSection("Tool Window");
    auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration;
    if (m_fPinToolWindow)
    {
        flags |= ImGuiWindowFlags_NoInputs;
    }
    ImGui::Begin(TOOL_WINDOW_NAME.data(), &m_fShowToolWindow, flags);

    DrawSettings(settings);

    m_translation.UseSection("Tool Window");

    ImGui::AlignTextToFramePadding();
    ImGui::Text(ICON_COD_MOVE);
    ImGui::SameLine();

    if (ImGui::Button(m_fPinToolWindow ? ICON_MD_PIN : ICON_MD_PIN_OUTLINE))
    {
        m_fPinToolWindow = true;
        // ImGui::GetIO().MouseDrawCursor = false;

        if (auto *const messageQueue = RE::UIMessageQueue::GetSingleton())
        {
            messageQueue->AddMessage(ToolWindowMenuName, RE::UI_MESSAGE_TYPE::kHide, nullptr);
        }
    }

    ImGui::SameLine();
    if (ImGui::Button(ICON_OCT_GEAR))
    {
        settings.appearance.showSettings = true;
    }
    ImGui::SetItemTooltip("%s", Translate("$Settings"));

    ImGui::SameLine();
    DrawInputMethodsCombo();

    ImGui::SameLine();
    if (State::GetInstance().Has(State::IN_ALPHANUMERIC))
    {
        ImGui::AlignTextToFramePadding();
        ImGui::Text("ENG");
        ImGui::SameLine();
    }
    ImGui::End();
}

static auto inactiveColor = ImVec4(0.45F, 0.45F, 0.45F, 0.9F);

void ImeUI::DrawSettings(Settings &settings)
{
    if (!settings.appearance.showSettings)
    {
        return;
    }
    auto      *imeManager = ImeController::GetInstance();
    const auto windowName = std::format("{}###SettingsWindow", m_translation.Get("$Settings"));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{});
    if (ImGui::Begin(windowName.c_str(), &settings.appearance.showSettings))
    {
        enum class Menu : int8_t
        {
            Appearance,
            FontBuilder,
            Behaviour
        };
        static auto currentMenu = Menu::Appearance;
        bool        appearing   = false;
        // Sidebar
        {
            ImGuiEx::StyleGuard styleGuard;
            styleGuard.Push(ImGuiEx::ColorHolder::Text(m_styles.colors.OnSurfaceVariant()))
                .Push(ImGuiEx::ColorHolder::ChildBg(m_styles.colors.SurfaceContainer()))
                .Push(ImGuiEx::ColorHolder::FrameBg(m_styles.colors.SurfaceContainer()))
                .Push(ImGuiEx::ColorHolder::FrameBgActive(m_styles.colors.Secondary()))
                .Push(ImGuiEx::ColorHolder::FrameBgHovered(m_styles.colors.SecondaryContainer()));

            if (ImGui::BeginChild(
                    "Sidebar", {ImGuiEx::M3::NavigationRail::Standard.width, -FLT_MIN}, ImGuiEx::ChildFlags().Borders()
                ))
            {
                ImGuiEx::M3::DrawNavMenu(ICON_MD_MENU);
                if (ImGuiEx::M3::DrawNavItem(
                        m_translation["$Appearance"], currentMenu == Menu::Appearance, ICON_MD_PALETTE, m_styles
                    ))
                {
                    currentMenu = Menu::Appearance;
                    appearing   = true;
                }
                if (ImGuiEx::M3::DrawNavItem(
                        m_translation["$Font_Builder"], currentMenu == Menu::FontBuilder, ICON_FA_WRENCH, m_styles
                    ))
                {
                    currentMenu = Menu::FontBuilder;
                    appearing   = true;
                }
                if (ImGuiEx::M3::DrawNavItem(
                        m_translation["$Behaviour"], currentMenu == Menu::Behaviour, ICON_OCT_GEAR, m_styles
                    ))
                {
                    currentMenu = Menu::Behaviour;
                    appearing   = true;
                }
            }
            ImGui::EndChild();
        }

        ImGui::SameLine(0, 0);
        m_translation.UseSection("Settings");
        ImGui::BeginGroup();
        switch (currentMenu)
        {
            case Menu::Appearance:
                DrawMenuAppearance(settings, appearing);
                break;
            case Menu::FontBuilder:
                DrawMenuFontBuilder(settings);
                break;
            case Menu::Behaviour:
                DrawMenuBehaviour(settings);
                break;
        }
        ImGui::EndGroup();
    }
    ImGui::End();
    ImGui::PopStyleVar();
    imeManager->SyncImeStateIfDirty();
}

void ImeUI::DrawMenuAppearance(Settings &settings, const bool appearing)
{
    m_panelAppearance.Draw(appearing);

    m_translation.UseSection("Settings");
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10, 4));
    if (ImGui::BeginTabBar("SettingsTabBar", ImGuiTabBarFlags_None))
    {
        DrawSettingsContent(settings);
        ImGui::EndTabBar();
    }
    ImGui::PopStyleVar();
    if (!settings.appearance.showSettings)
    {
        ImGuiUtil::ThemesLoader::Cleanup();
    }
}

void ImeUI::DrawMenuFontBuilder(Settings &settings)
{
    m_fontBuilderView.Draw(m_fontBuilder, m_translation, settings);
}

void ImeUI::DrawMenuBehaviour(Settings &settings) {}

void ImeUI::DrawModConfig(Settings &settings)
{
    bool enableMod = settings.enableMod;
    if (ImGui::Checkbox(Translate("$Enable_Mod"), &enableMod))
    {
        ImeController::GetInstance()->EnableMod(enableMod);
    }
    ImGui::SetItemTooltip("%s", Translate("$Enable_Mod_Tooltip"));

    DrawFontConfig(settings);

    if (DrawCombo(Translate("$Languages"), m_translateLanguages, settings.appearance.language))
    {
        m_translation.UseLanguage(settings.appearance.language.c_str());
    }

    if (ImGui::BeginCombo(Translate("$Themes"), settings.appearance.theme.c_str()))
    {
        size_t idx = 0;
        for (const auto &theme : m_themesLoader.GetThemes())
        {
            ImGui::PushID(static_cast<int>(idx));
            const bool isSelected = settings.appearance.themeIndex == idx;
            if (ImGui::Selectable(theme.name.c_str(), isSelected) && !isSelected)
            {
                ImGuiStyle style;
                if (m_themesLoader.UseTheme(idx, style))
                {
                    settings.appearance.themeIndex    = idx;
                    settings.appearance.theme         = theme.name;
                    settings.appearance.fontSizeScale = ImGui::GetStyle().FontScaleMain;
                    FillCommonStyleFields(style, settings);
                    ImGui::GetStyle() = style;
                }
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
            ImGui::PopID();
            idx++;
        }
        ImGui::EndCombo();
    }
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0F);
        ImGui::TextUnformatted("Themes provided by ImThemes (https://github.com/Patitotective/ImThemes)");
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void ImeUI::DrawFontConfig(Settings &settings) {}

void ImeUI::DrawFeatures(Settings &settings)
{
    DrawWindowPosUpdatePolicy(settings);

    ImGui::Checkbox(Translate("$Enable_Unicode_Paste"), &settings.input.enableUnicodePaste);
    ImGui::SetItemTooltip("%s", Translate("$Enable_Unicode_Paste_Tooltip"));

    ImGui::SameLine();
    if (ImGui::Checkbox(Translate("$Keep_Ime_Open"), &settings.input.keepImeOpen))
    {
        ImeController::GetInstance()->MarkDirty();
    }
    ImGui::SetItemTooltip("%s", Translate("$Keep_Ime_Open_Tooltip"));
}

void ImeUI::DrawSettingsContent(Settings &settings)
{
    if (ImGui::BeginTabItem(Translate("$Mod_Config")))
    {
        DrawModConfig(settings);
        ImGui::EndTabItem();
    }
    if (!settings.enableMod)
    {
        return;
    }
    if (ImGui::BeginTabItem(Translate("$Features")))
    {
        DrawStates();
        DrawFeatures(settings);
        ImGui::EndTabItem();
    }
}

auto ImeUI::DrawCombo(const char *label, const std::vector<std::string> &values, std::string &selected) -> bool
{
    bool clicked = false;
    if (ImGui::BeginCombo(label, selected.c_str()))
    {
        int32_t idx = 0;
        for (const auto &language : values)
        {
            ImGui::PushID(idx);
            const bool isSelected = selected == language;
            if (ImGui::Selectable(language.c_str(), isSelected) && !isSelected)
            {
                selected = language;
                clicked  = true;
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
            ImGui::PopID();
            idx++;
        }
        ImGui::EndCombo();
    }
    return clicked;
}

void ImeUI::DrawStates() const
{
    ImGui::SeparatorText(Translate("$States"));

    constexpr auto STATE_ACTIVE_COLOR = ImVec4(0.35F, 0.75F, 1.0F, 1.0F);
    const auto    &state              = State::GetInstance();
    ImGui::AlignTextToFramePadding();
    ImGui::TextColored(
        state.NotHas(State::IME_DISABLED) ? STATE_ACTIVE_COLOR : inactiveColor, "[ %s ]", ICON_FA_KEYBOARD
    );
    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", Translate("$IME"));
    ImGui::SetItemTooltip("%s", Translate("$Ime_Enabled_Tooltip"));

    ImGui::SameLine();

    ImGui::AlignTextToFramePadding();
    ImGui::TextColored(m_pImeWnd->IsFocused() ? STATE_ACTIVE_COLOR : inactiveColor, "[ %s ]", ICON_FA_CROSSHAIR);
    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", Translate("$FOCUS"));
    ImGui::SetItemTooltip("%s", Translate("$Ime_Focus_Tooltip"));

    ImGui::SameLine(0, ImGui::GetFontSize());
    ImGui::TextDisabled("|");
    ImGui::SameLine(0, ImGui::GetFontSize());
    if (ImGui::Button(Translate("$Force_Focus_Ime")))
    {
        ImeController::GetInstance()->ForceFocusIme();
    }
#ifdef SIMPLE_IME_DEBUG
    auto action = [&state, &STATE_ACTIVE_COLOR](const State::StateKey stateKey) {
        ImGui::SameLine();
        ImGui::TextColored(state.Has(stateKey) ? STATE_ACTIVE_COLOR : inactiveColor, "[ %s ]", ICON_FA_CROSSHAIR);
    };
    ImGui::Text("IN_COMPOSING: ");
    action(State::IN_COMPOSING);
    ImGui::Text("IN_CAND_CHOOSING: ");
    action(State::IN_CAND_CHOOSING);
    ImGui::Text("IN_ALPHANUMERIC: ");
    action(State::IN_ALPHANUMERIC);
    ImGui::Text("IME_OPEN: ");
    action(State::IME_OPEN);
    ImGui::Text("LANG_PROFILE_ACTIVATED: ");
    action(State::LANG_PROFILE_ACTIVATED);
    ImGui::Text("IME_DISABLED: ");
    action(State::IME_DISABLED);
    ImGui::Text("TSF_FOCUS: ");
    action(State::TSF_FOCUS);
    ImGui::Text("GAME_LOADING: ");
    action(State::GAME_LOADING);
    ImGui::Text("KEYBOARD_OPEN: ");
    action(State::KEYBOARD_OPEN);
#endif
}

template <typename T>
static constexpr auto RadioButton(const char *label, T *pValue, T value) -> bool
{
    const bool pressed = ImGui::RadioButton(label, *pValue == value);
    if (pressed)
    {
        *pValue = value;
    }
    return pressed;
}

void ImeUI::DrawWindowPosUpdatePolicy(Settings &settings)
{
    using Policy = Settings::WindowPosUpdatePolicy;
    m_translation.UseSection("Ime Window Pos");
    {
        ImGui::SeparatorText(Translate("$Policy"));

        RadioButton(Translate("$Update_By_Cursor"), &settings.input.posUpdatePolicy, Policy::BASED_ON_CURSOR);
        ImGui::SetItemTooltip("%s", Translate("$Update_By_Cursor_Tooltip"));

        ImGui::SameLine();
        RadioButton(Translate("$Update_By_Caret"), &settings.input.posUpdatePolicy, Policy::BASED_ON_CARET);
        ImGui::SetItemTooltip("%s", Translate("$Update_By_Caret_Tooltip"));

        ImGui::SameLine();
        RadioButton(Translate("$Update_By_None"), &settings.input.posUpdatePolicy, Policy::NONE);
        ImGui::SetItemTooltip("%s", Translate("$Update_By_None_Tooltip"));
    }
    m_translation.UseSection("Settings");
}

void ImeUI::DrawCompWindow(const Settings &settings) const
{
    static float CursorAnim = 0.F;

    CursorAnim += ImGui::GetIO().DeltaTime;

    const ImVec4 highLightText = ImGui::GetStyle().Colors[ImGuiCol_TextLink];
    const auto  &textEditor    = m_pTextService->GetTextEditor();

    const auto &editorText = textEditor.GetText();
    LONG        acpStart   = 0;
    LONG        acpEnd     = 0;
    textEditor.GetSelection(&acpStart, &acpEnd);

    bool success = true;

    const size_t textSize = editorText.size();
    acpStart              = std::max(0L, acpStart);
    acpEnd                = std::max(0L, acpEnd);

    acpStart = std::min(static_cast<long>(textSize), acpStart);
    acpEnd   = std::min(static_cast<long>(textSize), acpEnd);

    if (acpEnd < acpStart)
    {
        std::swap(acpStart, acpEnd);
    }

    ImGui::PushStyleVarX(ImGuiStyleVar_ItemSpacing, 0);
    if (acpStart > 0)
    {
        std::string startToCaret;
        success = WCharUtils::ToString(editorText.c_str(), acpStart, startToCaret);
        if (success)
        {
            ImGui::TextColored(highLightText, "%s", startToCaret.c_str());
        }
    }

    // caret
    ImGui::SameLine();
    if (fmodf(CursorAnim, 1.2F) <= 0.8F)
    {
        ImVec2 const cursorScreenPos = ImGui::GetCursorScreenPos();
        ImVec2 const min(cursorScreenPos.x, cursorScreenPos.y + 0.5f);
        ImGui::GetWindowDrawList()->AddLine(
            min,
            ImVec2(min.x, cursorScreenPos.y + ImGui::GetFontSize() - 1.5f),
            ImGui::GetColorU32(ImGuiCol_InputTextCursor),
            1.0f * settings.state.dpiScale
        );
    }

    success = success && static_cast<size_t>(acpStart) < editorText.size();
    if (success)
    {
        std::string caretToEnd;
        auto        subFromCaret = editorText.substr(acpStart);
        success = WCharUtils::ToString(subFromCaret.c_str(), static_cast<int>(subFromCaret.size()), caretToEnd);
        if (success)
        {
            ImGui::TextColored(highLightText, "%s", caretToEnd.c_str());
        }
    }
    if (!success)
    {
        ImGui::NewLine();
    }
    ImGui::PopStyleVar();
}

class ImGuiStyleVarScope
{
    int count = 0;

public:
    void Push(ImGuiStyleVar var, const ImVec2 &value)
    {
        ImGui::PushStyleVar(var, value);
        count++;
    }

    ~ImGuiStyleVarScope()
    {
        ImGui::PopStyleVar(count);
    }
};

struct ImGuiColorScope
{
    ImGuiColorScope(const ImGuiCol colorIndex, const ImU32 color)
    {
        ImGui::PushStyleColor(colorIndex, color);
    }

    ImGuiColorScope(const ImGuiCol colorIndex, const ImVec4 color)
    {
        ImGui::PushStyleColor(colorIndex, color);
    }

    ~ImGuiColorScope()
    {
        ImGui::PopStyleColor();
    }
};

void ImeUI::DrawCandidateWindows() const
{
    const auto  &candidateUi   = m_pTextService->GetCandidateUi();
    static DWORD hovered       = ULONG_MAX;
    const auto   candidateList = candidateUi.CandidateList();
    if (!candidateList.empty())
    {

        DWORD              index      = 0;
        DWORD              clicked    = candidateList.size();
        DWORD              anyHovered = candidateList.size();
        ImGuiStyleVarScope styleVarScope;
        styleVarScope.Push(ImGuiStyleVar_FramePadding, {5, 2});
        styleVarScope.Push(ImGuiStyleVar_ItemSpacing, {0, 0});

        auto           &style = ImGui::GetStyle();
        ImGuiColorScope buttonColorScope(ImGuiCol_Button, 0);
        for (const auto &candidate : candidateList)
        {
            ImGui::PushID(static_cast<int>(index));
            std::optional<ImGuiColorScope> buttonColorScope1;
            std::optional<ImGuiColorScope> textColorScope;
            if (hovered == index)
            {
                buttonColorScope1.emplace(ImGuiCol_Button, style.Colors[ImGuiCol_Button]);
            }
            if (index == candidateUi.Selection())
            {
                textColorScope.emplace(ImGuiCol_Text, style.Colors[ImGuiCol_TextLink]);
            }
            if (ImGui::Button(candidate.c_str()))
            {
                clicked = index;
            }
            buttonColorScope1.reset();
            textColorScope.reset();
            if (ImGui::IsItemHovered())
            {
                anyHovered = index;
            }
            ImGui::SameLine();
            ImGui::PopID();
            index++;
        }
        hovered = anyHovered;
        if (clicked < candidateList.size())
        {
            TaskQueue::GetInstance().AddImeThreadTask([this, clicked] {
                m_pTextService->CommitCandidate(m_pImeWnd->GetHWND(), clicked);
            });
            PostMessageA(m_pImeWnd->GetHWND(), CM_EXECUTE_TASK, 0, 0);
        }
    }
}

inline auto ImeUI::Translate(const char *label) const -> const char *
{
    return m_translation.Get(label);
}

auto ImeUI::UpdateImeWindowPos(const Settings &settings, ImVec2 &windowPos) -> void
{
    switch (settings.input.posUpdatePolicy)
    {
        case Settings::WindowPosUpdatePolicy::BASED_ON_CURSOR:
            if (const auto *cursor = RE::MenuCursor::GetSingleton(); cursor != nullptr)
            {
                windowPos.x = cursor->cursorPosX;
                windowPos.y = cursor->cursorPosY;
            }
            break;
        case Settings::WindowPosUpdatePolicy::BASED_ON_CARET: {
            FocusGFxCharacterInfo::GetInstance().UpdateCaretCharBoundaries();
            UpdateImeWindowPosByCaret(windowPos);
            break;
        }
        default:;
    }
}

auto ImeUI::UpdateImeWindowPosByCaret(ImVec2 &windowPos) -> void
{
    const auto &instance       = FocusGFxCharacterInfo::GetInstance();
    const auto &charBoundaries = instance.CharBoundaries();

    windowPos.x = charBoundaries.left;
    windowPos.y = charBoundaries.bottom;
}

auto ImeUI::IsImeNeedRelayout() const -> bool
{
    const auto &viewport = ImGui::GetMainViewport();

    return m_imeWindowPos.x + m_imeWindowSize.x > viewport->Size.x + viewport->Pos.x ||
           m_imeWindowPos.y + m_imeWindowSize.y > viewport->Size.y + viewport->Pos.y;
}

void ImeUI::ClampWindowToViewport(const ImVec2 &windowSize, ImVec2 &windowPos)
{
    const auto &viewport   = ImGui::GetMainViewport();
    const float viewRight  = viewport->Pos.x + viewport->Size.x;
    const float viewBottom = viewport->Pos.y + viewport->Size.y;
    windowPos.x            = std::max(windowPos.x, viewport->Pos.x);
    windowPos.y            = std::max(windowPos.y, viewport->Pos.y);

    if (windowSize.x < viewport->Size.x && windowPos.x + windowSize.x > viewRight)
    {
        windowPos.x = viewRight - windowSize.x;
    }
    if (windowSize.y < viewport->Size.y && windowPos.y + windowSize.y > viewBottom)
    {
        windowPos.y = viewBottom - windowSize.y;
    }

    const ImVec2 min            = windowPos;
    const ImVec2 max            = {windowPos.x + windowSize.x, windowPos.y + windowSize.y};
    const auto  &instance       = FocusGFxCharacterInfo::GetInstance();
    const auto  &charBoundaries = instance.CharBoundaries();
    if (charBoundaries.top < max.y && charBoundaries.bottom > min.y && charBoundaries.left < max.x &&
        charBoundaries.right > min.x) // is overlaps?
    {
        // Move the window above the boundary
        const float newY = charBoundaries.top - windowSize.y;
        if (newY >= viewport->Pos.y)
        {
            windowPos.y = newY;
        }
    }
}

void ImeUI::FillCommonStyleFields(ImGuiStyle &style, const Settings &settings)
{
    if (settings.state.dpiScale != 1.0F)
    {
        style.ScaleAllSizes(settings.state.dpiScale);
    }
    style.FontScaleDpi = settings.state.dpiScale;
}

} // namespace  SimpleIME
} // namespace LIBC_NAMESPACE_DECL