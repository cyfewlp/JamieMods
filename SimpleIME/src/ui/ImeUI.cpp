//
// Created by jamie on 25-1-21.
//

#define IMGUI_DEFINE_MATH_OPERATORS

#include "ImeUI.h"

#include "ImeWnd.hpp"
#include "core/State.h"
#include "i18n/TranslatorHolder.h"
#include "icons.h"
#include "ime/ImeController.h"
#include "imgui.h"
#include "imguiex/ImGuiEx.h"
#include "imguiex/Material3.h"
#include "imguiex/imguiex_enum_wrap.h"
#include "imguiex/imguiex_m3.h"
#include "imguiex/m3/facade/others.h"
#include "log.h"
#include "ui/Settings.h"

#include <cstdint>
#include <format>
#include <string>
#include <utility>

#pragma comment(lib, "dwrite.lib")

namespace Ime
{
void ImeUI::Initialize()
{
    logger::debug("Initializing ImeUI...");
    m_fontBuilder.Initialize();
}

void ImeUI::ApplySettings(Settings::Appearance &appearance)
{
    m_panelAppearance.ApplySettings(appearance);
}

void ImeUI::DrawSettings(Settings &settings)
{
    if (!settings.appearance.showSettings)
    {
        return;
    }
    auto      *imeManager = ImeController::GetInstance();
    const auto windowName = std::format("{}###SettingsWindow", Translate("Settings.Settings"));

    const ImVec2 &viewportSize = ImGui::GetMainViewport()->Size;
    ImGui::SetNextWindowSize(viewportSize * 0.5F, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(viewportSize * 0.25F, ImGuiCond_FirstUseEver);
    if (ImGui::Begin(windowName.c_str(), &settings.appearance.showSettings, ImGuiEx::WindowFlags().NoTitleBar()))
    {
        if (auto appBar = ImGuiEx::M3::AppBar(); appBar)
        {
            appBar.Title("SimpleIME", "Created By Jamie");
            if (appBar.TrailingIcon(ICON_X))
            {
                settings.appearance.showSettings = false;
            }
            if (auto &m3Styles = ImGuiEx::M3::Context::GetM3Styles();
                appBar.TrailingIcon(m3Styles.Colors().IsDark() ? std::string_view(ICON_MOON) : ICON_SUN))
            {
                m3Styles.ToggleLightDarkScheme();
                ImGuiEx::M3::SetupDefaultImGuiStyles(ImGui::GetStyle());
            }
        }

        enum class Menu : int8_t
        {
            Appearance,
            FontBuilder,
            Behaviour
        };
        static auto currentMenu = std::pair{Menu::Appearance, true};

        // Sidebar
        {
            if (ImGuiEx::M3::BeginResponsiveNavRail("Sidebar"))
            {
                if (ImGuiEx::M3::NavItem(Translate("Settings.Sidebar.Appearance"), currentMenu.first == Menu::Appearance, ICON_PALETTE))
                {
                    currentMenu = {Menu::Appearance, true};
                }
                if (ImGuiEx::M3::NavItem(Translate("Settings.Sidebar.FontBuilder"), currentMenu.first == Menu::FontBuilder, ICON_CASE_UPPER))
                {
                    currentMenu = {Menu::FontBuilder, true};
                }
                if (ImGuiEx::M3::NavItem(Translate("Settings.Sidebar.Behaviour"), currentMenu.first == Menu::Behaviour, ICON_SETTINGS))
                {
                    currentMenu = {Menu::Behaviour, true};
                }
                ImGuiEx::M3::EndNavRail();
            }
        }

        ImGui::SameLine(0, 0);

        ImGui::BeginGroup();
        switch (currentMenu.first)
        {
            case Menu::Appearance:
                DrawMenuAppearance(settings);
                break;
            case Menu::FontBuilder:
                DrawMenuFontBuilder(settings);
                break;
            case Menu::Behaviour:
                DrawMenuBehaviour(settings);
                break;
        }
        ImGui::EndGroup();
        currentMenu.second = false;
    }
    ImGui::End();
    imeManager->SyncImeStateIfDirty();
}

void ImeUI::DrawMenuAppearance(Settings &settings)
{
    m_panelAppearance.Draw(settings);

    // sync theme config
    const auto &m3Styles                               = ImGuiEx::M3::Context::GetM3Styles();
    const auto &[contrastLevel, sourceColor, darkMode] = m3Styles.Colors().GetSchemeConfig();
    settings.appearance.themeSourceColor               = sourceColor;
    settings.appearance.themeDarkMode                  = darkMode;
    settings.appearance.themeContrastLevel             = contrastLevel;
}

void ImeUI::DrawMenuFontBuilder(Settings &settings)
{
    m_fontBuilderView.Draw(m_fontBuilder, settings);
}

void ImeUI::DrawMenuBehaviour(Settings &settings) const
{
    bool enableMod = settings.enableMod;
    if (ImGui::Checkbox(Translate("Settings.Behaviour.EnableMod"), &enableMod))
    {
        ImeController::GetInstance()->EnableMod(enableMod);
    }
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.Behaviour.EnableModToolTip"));
    if (!settings.enableMod)
    {
        return;
    }

    DrawStates();
    DrawFeatures(settings);
}

void ImeUI::DrawFeatures(Settings &settings)
{
    DrawWindowPosUpdatePolicy(settings);

    ImGui::Checkbox(Translate("Settings.Behaviour.EnableUnicodePaste"), &settings.input.enableUnicodePaste);
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.Behaviour.EnableUnicodePasteTooltip"));

    ImGui::SameLine();
    if (ImGui::Checkbox(Translate("Settings.Behaviour.KeepImeOpen"), &settings.input.keepImeOpen))
    {
        ImeController::GetInstance()->MarkDirty();
    }
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.Behaviour.KeepImeOpenTooltip"));
}

void ImeUI::DrawStates() const
{
    ImGui::SeparatorText(Translate("Settings.Behaviour.States"));

    const auto &state = State::GetInstance();
    ImGuiEx::M3::XSmallIcon(ICON_KEYBOARD);
    ImGui::SameLine();
    ImGui::Text("%s", Translate("Settings.Behaviour.ImeEnabled"));
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.Behaviour.ImeEnabledTooltip"));

    ImGui::SameLine();

    ImGuiEx::M3::XSmallIcon(ICON_FOCUS);
    ImGui::SameLine();
    ImGui::Text("%s", Translate("Settings.Behaviour.Focus"));
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.Behaviour.FocusTooltip"));

    ImGui::SameLine(0, ImGui::GetFontSize());
    ImGui::TextDisabled("|");
    ImGui::SameLine(0, ImGui::GetFontSize());
    if (ImGui::Button(Translate("Settings.Behaviour.ForceFocusIme")))
    {
        ImeController::GetInstance()->ForceFocusIme();
    }
#ifdef DEBUG
    auto action = [&state](const State::StateKey stateKey) -> void {
        ImGui::SameLine();
        if (state.Has(stateKey))
        {
            ImGuiEx::M3::Icon(ICON_EYE, ImGuiEx::M3::Spec::SizeTips::SMALL);
        }
        else
        {
            ImGuiEx::M3::Icon(ICON_EYE_OFF, ImGuiEx::M3::Spec::SizeTips::SMALL);
        }
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

    ImGui::SeparatorText(Translate("Settings.Behaviour.ImePos.Policy"));

    RadioButton(Translate("Settings.Behaviour.ImePos.UpdateByCursor"), &settings.input.posUpdatePolicy, Policy::BASED_ON_CURSOR);
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.Behaviour.ImePos.UpdateByCursorTooltip"));

    ImGui::SameLine();
    RadioButton(Translate("Settings.Behaviour.ImePos.UpdateByCaret"), &settings.input.posUpdatePolicy, Policy::BASED_ON_CARET);
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.Behaviour.ImePos.UpdateByCaretTooltip"));

    ImGui::SameLine();
    RadioButton(Translate("Settings.Behaviour.ImePos.UpdateByNone"), &settings.input.posUpdatePolicy, Policy::NONE);
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.Behaviour.ImePos.UpdateByNoneTooltip"));
}

} // namespace Ime
