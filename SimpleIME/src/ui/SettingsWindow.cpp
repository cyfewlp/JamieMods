//
// Created by jamie on 2026/3/10.
//

#include "ui/SettingsWindow.h"

#include "i18n/translator_manager.h"
#include "icons.h"
#include "ime/ImeController.h"
#include "imguiex/imguiex_enum_wrap.h"
#include "imguiex/imguiex_m3.h"
#include "imguiex/m3/spec/layout.h"

namespace Ime::UI
{

void SettingsWindow::Draw(Settings &settings)
{
    constexpr ImVec2 CENTER_ALIGN_PIVOT(0.5F, 0.5F);
    constexpr float  DEFAULT_WINDOW_HEIGHT_FACTOR = 0.75F;

    const auto &viewport            = ImGui::GetMainViewport();
    auto       &m3Styles            = ImGuiEx::M3::Context::GetM3Styles();
    // At least two panel(font builder). So, the minimum width compute from Expanded layout.
    const float minWidth            = m3Styles.GetPixels(M3Spec::Layout::Expanded::Breakpoint);
    const float maxWidth            = m3Styles.GetPixels(M3Spec::Layout::Large::Breakpoint);
    const float defaultWindowHeight = viewport->Size.y * DEFAULT_WINDOW_HEIGHT_FACTOR;

    ImGui::SetNextWindowSize({minWidth, defaultWindowHeight}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_FirstUseEver, CENTER_ALIGN_PIVOT);

    ImGui::SetNextWindowSizeConstraints({minWidth, 0.0F}, {maxWidth, FLT_MAX});
    if (ImGui::Begin("SettingsWindow", &settings.appearance.showSettings, ImGuiEx::WindowFlags().NoTitleBar()))
    {
        if (auto appBar = ImGuiEx::M3::AppBar(); appBar)
        {
            appBar.LeadingIcon(ICON_GITHUB);
            appBar.Title("SimpleIME", "Created By Jamie");
            if (appBar.TrailingIcon(ICON_X))
            {
                settings.appearance.showSettings = false;
            }
            if (appBar.TrailingIcon(m3Styles.Colors().IsDark() ? std::string_view(ICON_MOON) : ICON_SUN))
            {
                m3Styles.ToggleLightDarkScheme();
                ImGuiEx::M3::SetupDefaultImGuiStyles(ImGui::GetStyle());
            }
        }

        // Sidebar
        if (ImGuiEx::M3::BeginResponsiveNavRail("Sidebar"))
        {
            if (ImGuiEx::M3::NavItem(Translate("Settings.Sidebar.Appearance"), m_currentMenu == Menu::Appearance, ICON_PALETTE))
            {
                m_currentMenu = Menu::Appearance;
            }
            if (ImGuiEx::M3::NavItem(Translate("Settings.Sidebar.FontBuilder"), m_currentMenu == Menu::FontBuilder, ICON_CASE_UPPER))
            {
                m_currentMenu = Menu::FontBuilder;
            }
            if (ImGuiEx::M3::NavItem(Translate("Settings.Sidebar.Behaviour"), m_currentMenu == Menu::Behaviour, ICON_SETTINGS))
            {
                m_currentMenu = Menu::Behaviour;
            }
            ImGuiEx::M3::EndNavRail();
        }

        ImGui::SameLine(0, 0);

        ImGui::BeginGroup();
        switch (m_currentMenu)
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
    }
    ImGui::End();
    ImeController::GetInstance()->SyncImeStateIfDirty();
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
    auto      &m3Styles   = ImGuiEx::M3::Context::GetM3Styles();
    const auto styleGuard = ImGuiEx::StyleGuard().Color<ImGuiCol_ChildBg>(m3Styles.Colors()[M3Spec::ColorRole::surfaceContainerLowest]);

    if (ImGui::BeginChild("Behaviour"))
    {
        bool enableMod = settings.enableMod;
        if (ImGuiEx::M3::Checkbox(Translate("Settings.Behaviour.EnableMod"), enableMod, ICON_CHECK))
        {
            ImeController::GetInstance()->EnableMod(enableMod);
        }
        ImGuiEx::M3::SetItemToolTip(Translate("Settings.Behaviour.EnableModToolTip"));
        if (!settings.enableMod)
        {
            return;
        }

        (void)ImGuiEx::M3::Checkbox(
            Translate("Settings.Behaviour.FixInconsistentTextEntryCount"), settings.fixInconsistentTextEntryCount, ICON_CHECK
        );
        ImGuiEx::M3::SetItemToolTip(Translate("Settings.Behaviour.FixInconsistentTextEntryCountToolTip"));

        DrawStates();
        DrawFeatures(settings);
    }
    ImGui::EndChild();
}

void SettingsWindow::DrawFeatures(Settings &settings)
{
    DrawWindowPosUpdatePolicy(settings);

    ImGuiEx::M3::Checkbox(Translate("Settings.Behaviour.EnableUnicodePaste"), settings.input.enableUnicodePaste, ICON_CHECK);
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.Behaviour.EnableUnicodePasteToolTip"));

    ImGui::SameLine();
    if (ImGuiEx::M3::Checkbox(Translate(Translate("Settings.Behaviour.KeepImeOpen")), settings.input.keepImeOpen, ICON_CHECK))
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

    const auto &state     = Core::State::GetInstance();
    auto        StateIcon = [](bool enable) constexpr -> void {
        if (enable)
        {
            ImGuiEx::M3::Icon(ICON_EYE, ImGuiEx::M3::Spec::SizeTips::SMALL);
        }
        else
        {
            ImGuiEx::M3::Icon(ICON_EYE_OFF, ImGuiEx::M3::Spec::SizeTips::SMALL);
        }
    };

    StateIcon(state.NotHas(Core::State::IME_DISABLED));
    ImGui::SameLine();
    ImGuiEx::M3::AlignedLabel(Translate("Settings.Behaviour.ImeEnabled"));
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.Behaviour.ImeEnabledTooltip"));

    const auto TextServiceFocused = state.Has(Core::State::TEXT_SERVICE_FOCUS);
    StateIcon(TextServiceFocused);
    ImGui::SameLine();
    ImGuiEx::M3::AlignedLabel(Translate("Settings.Behaviour.Focus"));
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.Behaviour.FocusTooltip"));

    ImGui::SameLine(0, ImGui::GetFontSize());
    ImGui::BeginDisabled(TextServiceFocused);
    if (ImGuiEx::M3::XSmallButton(Translate("Settings.Behaviour.ForceFocusIme"), ""))
    {
        ImeController::GetInstance()->ForceFocusIme();
    }
    ImGui::EndDisabled();
#ifdef DEBUG
    // clang-format off
    StateIcon(state.Has(Core::State::IN_COMPOSING));        ImGui::SameLine(); ImGuiEx::M3::AlignedLabel("IN_COMPOSING");
    StateIcon(state.Has(Core::State::IN_CAND_CHOOSING));    ImGui::SameLine(); ImGuiEx::M3::AlignedLabel("IN_CAND_CHOOSING");
    StateIcon(state.Has(Core::State::IN_ALPHANUMERIC));     ImGui::SameLine(); ImGuiEx::M3::AlignedLabel("IN_ALPHANUMERIC");
    StateIcon(state.Has(Core::State::IME_OPEN));            ImGui::SameLine(); ImGuiEx::M3::AlignedLabel("IME_OPEN");
    StateIcon(state.Has(Core::State::LANG_PROFILE_ACTIVATED)); ImGui::SameLine(); ImGuiEx::M3::AlignedLabel("LANG_PROFILE_ACTIVATED");
    StateIcon(state.Has(Core::State::IME_DISABLED));        ImGui::SameLine(); ImGuiEx::M3::AlignedLabel("IME_DISABLED");
    StateIcon(state.Has(Core::State::GAME_LOADING));        ImGui::SameLine(); ImGuiEx::M3::AlignedLabel("GAME_LOADING");
    StateIcon(state.Has(Core::State::KEYBOARD_OPEN));       ImGui::SameLine(); ImGuiEx::M3::AlignedLabel("KEYBOARD_OPEN");
    // clang-format on
#endif
}
} // namespace Ime::UI
