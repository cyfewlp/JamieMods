//
// Created by jamie on 2026/2/6.
//

#include "ui/LanguageBar.h"

#include "core/State.h"
#include "i18n/translator_manager.h"
#include "icons.h"
#include "ime/ImeController.h"
#include "imgui.h"
#include "imguiex/ImGuiEx.h"
#include "imguiex/imguiex_enum_wrap.h"
#include "imguiex/imguiex_m3.h"
#include "imguiex/m3/facade/button_groups.h"
#include "menu/MenuNames.h"
#include "tsf/ConversionModeUtil.h"
#include "tsf/LangProfile.h"
#include "ui/ToolWindow.h"

namespace Ime
{

namespace
{
constexpr auto LANGUAGE_BAR              = "LanguageBar";
constexpr auto LanguageProfilesMenuTitle = "##LanguageProfiles";

void DrawInputMethodsCombo(const LangProfile &activeLangProfile, const std::vector<LangProfile> &langProfiles)
{
    uint32_t clickedIndex = UINT32_MAX;
    uint32_t idx          = 0;
    if (ImGuiEx::M3::BeginMenu(LanguageProfilesMenuTitle, ImGui::GetItemRectMin(), ImGui::GetItemRectMax()))
    {
        for (const auto &langProfile : langProfiles)
        {
            ImGui::PushID(static_cast<int>(idx));
            ImGuiEx::M3::MenuItemConfiguration config{};
            config.supportingText = langProfile.desc;
            const bool isSelected = IsEqualGUID(activeLangProfile.guidProfile, langProfile.guidProfile) == TRUE;
            if (ImGuiEx::M3::MenuItem(langProfile.localeDisplayName, isSelected, config) && !isSelected)
            {
                clickedIndex = idx;
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
            ImGui::PopID();
            idx++;
        }
        ImGuiEx::M3::EndMenu();
    }
    if (clickedIndex != UINT32_MAX)
    {
        ImeController::GetInstance()->ActivateLangProfile(langProfiles[clickedIndex].guidProfile);
    }
}
} // namespace

namespace UI::LanguageBar
{
auto Draw(bool &pinned, bool &toolWindowShowing, const LangProfile &activeLangProfile, const std::vector<LangProfile> &langProfiles) -> void
{
    auto flags = ImGuiEx::WindowFlags().AlwaysAutoResize().NoNav().NoDecoration();
    if (pinned)
    {
        flags = flags.NoInputs();
    }
    if (ImGuiEx::M3::BeginFloatingToolbar(LANGUAGE_BAR, nullptr, M3Spec::ToolBarColors::Standard, flags))
    {
        ImGuiEx::M3::SmallIcon(ICON_MOVE);
        ImGui::SameLine();

        constexpr auto iconButtonColors = ImGuiEx::M3::Spec::IconButtonColors::Standard;
        if (ImGuiEx::M3::SmallIconButton(pinned ? static_cast<std::string_view>(ICON_PIN_OFF) : ICON_PIN, iconButtonColors))
        {
            pinned = true;
        }

        ImGui::SameLine();

        if (ImGuiEx::M3::SmallIconButton(ICON_SETTINGS, iconButtonColors))
        {
            toolWindowShowing = true;
        }
        ImGuiEx::M3::SetItemToolTip(Translate("Settings.Settings"));

        ImGui::SameLine();

        auto      &state          = Core::State::GetInstance();
        auto      &conversionMode = state.GetConversionMode();
        const auto cModeName      = GetConversionModeNameShort(activeLangProfile.langid, conversionMode, state.IsKeyboardOpen());
        if (!cModeName.empty())
        {
            ImGuiEx::M3::AlignedLabel(cModeName);
            ImGuiEx::M3::SameLine(0.F, M3Spec::StandardSmallButtonGroup::BetweenSpace);
        }

        ImGuiEx::M3::MenuItemConfiguration config{};
        config.supportingText = activeLangProfile.desc;
        if (ImGuiEx::M3::MenuItem(activeLangProfile.language, false, config))
        {
            ImGui::OpenPopup(LanguageProfilesMenuTitle);
        }
        DrawInputMethodsCombo(activeLangProfile, langProfiles);

        ImGuiEx::M3::EndFloatingToolbar();
    }
}
} // namespace UI::LanguageBar
} // namespace Ime
