//
// Created by jamie on 2026/3/10.
//

#include "ui/SettingsWindow.h"

#include "i18n/translator_manager.h"
#include "icons.h"
#include "ime/ImeController.h"
#include "imguiex/imguiex_enum_wrap.h"
#include "imguiex/imguiex_m3.h"

namespace Ime::UI
{

void SettingsWindow::Draw(Settings &settings)
{
    auto      *imeManager = ImeController::GetInstance();
    const auto windowName = std::format("{}###SettingsWindow", Translate("Settings.Settings"));

    const ImVec2 &viewportSize = ImGui::GetMainViewport()->Size;
    ImGui::SetNextWindowSize({viewportSize.x * 0.5F, viewportSize.y * 0.5F}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos({viewportSize.x * 0.25F, viewportSize.y * 0.25F}, ImGuiCond_FirstUseEver);
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

void SettingsWindow::DrawMenuAppearance(Settings &settings)
{
    m_panelAppearance.Draw(settings);

    // sync theme config
    const auto &m3Styles                               = ImGuiEx::M3::Context::GetM3Styles();
    const auto &[contrastLevel, sourceColor, darkMode] = m3Styles.Colors().GetSchemeConfig();
    settings.appearance.schemeConfig.sourceColor       = sourceColor;
    settings.appearance.schemeConfig.darkMode          = darkMode;
    settings.appearance.schemeConfig.contrastLevel     = contrastLevel;
}

void SettingsWindow::DrawMenuFontBuilder(Settings &settings)
{
    m_fontBuilderView.Draw(m_fontBuilder, settings);
}

void SettingsWindow::DrawMenuBehaviour(Settings &settings) const
{
    bool enableMod = settings.enableMod;
    if (ImGui::Checkbox(Translate("Settings.Behaviour.EnableMod").data(), &enableMod))
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

void SettingsWindow::DrawFeatures(Settings &settings)
{
    DrawWindowPosUpdatePolicy(settings);

    ImGui::Checkbox(Translate("Settings.Behaviour.EnableUnicodePaste").data(), &settings.input.enableUnicodePaste);
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.Behaviour.EnableUnicodePasteTooltip"));

    ImGui::SameLine();
    if (ImGui::Checkbox(Translate("Settings.Behaviour.KeepImeOpen").data(), &settings.input.keepImeOpen))
    {
        ImeController::GetInstance()->MarkDirty();
    }
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.Behaviour.KeepImeOpenTooltip"));
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

void SettingsWindow::DrawWindowPosUpdatePolicy(Settings &settings)
{
    using Policy = Settings::WindowPosUpdatePolicy;

    ImGui::SeparatorText(Translate("Settings.Behaviour.ImePos.Policy").data());

    RadioButton(Translate("Settings.Behaviour.ImePos.UpdateByCursor").data(), &settings.input.posUpdatePolicy, Policy::BASED_ON_CURSOR);
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.Behaviour.ImePos.UpdateByCursorTooltip"));

    ImGui::SameLine();
    RadioButton(Translate("Settings.Behaviour.ImePos.UpdateByCaret").data(), &settings.input.posUpdatePolicy, Policy::BASED_ON_CARET);
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.Behaviour.ImePos.UpdateByCaretTooltip"));

    ImGui::SameLine();
    RadioButton(Translate("Settings.Behaviour.ImePos.UpdateByNone").data(), &settings.input.posUpdatePolicy, Policy::NONE);
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.Behaviour.ImePos.UpdateByNoneTooltip"));
}

void SettingsWindow::DrawStates() const
{
    ImGui::SeparatorText(Translate("Settings.Behaviour.States").data());

    const auto &state = Core::State::GetInstance();
    ImGuiEx::M3::XSmallIcon(ICON_KEYBOARD);
    ImGui::SameLine();
    ImGuiEx::M3::TextUnformatted(Translate("Settings.Behaviour.ImeEnabled"));
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.Behaviour.ImeEnabledTooltip"));

    ImGui::SameLine();

    ImGuiEx::M3::XSmallIcon(ICON_FOCUS);
    ImGui::SameLine();
    ImGuiEx::M3::TextUnformatted(Translate("Settings.Behaviour.Focus"));
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.Behaviour.FocusTooltip"));

    ImGui::SameLine(0, ImGui::GetFontSize());
    ImGui::TextDisabled("|");
    ImGui::SameLine(0, ImGui::GetFontSize());
    if (ImGuiEx::M3::SmallButton(Translate("Settings.Behaviour.ForceFocusIme"), ""))
    {
        ImeController::GetInstance()->ForceFocusIme();
    }
#ifdef DEBUG
    auto action = [&state](const Core::State::StateKey stateKey) -> void {
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
    action(Core::State::IN_COMPOSING);
    ImGui::Text("IN_CAND_CHOOSING: ");
    action(Core::State::IN_CAND_CHOOSING);
    ImGui::Text("IN_ALPHANUMERIC: ");
    action(Core::State::IN_ALPHANUMERIC);
    ImGui::Text("IME_OPEN: ");
    action(Core::State::IME_OPEN);
    ImGui::Text("LANG_PROFILE_ACTIVATED: ");
    action(Core::State::LANG_PROFILE_ACTIVATED);
    ImGui::Text("IME_DISABLED: ");
    action(Core::State::IME_DISABLED);
    ImGui::Text("TSF_FOCUS: ");
    action(Core::State::TSF_FOCUS);
    ImGui::Text("GAME_LOADING: ");
    action(Core::State::GAME_LOADING);
    ImGui::Text("KEYBOARD_OPEN: ");
    action(Core::State::KEYBOARD_OPEN);
#endif
}
} // namespace Ime::UI
