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

#include <RE/U/UIMessage.h>
#include <RE/U/UIMessageQueue.h>

namespace Ime
{

namespace
{
constexpr auto LANGUAGE_BAR = "LanguageBar";

void DrawInputMethodsCombo(const LangProfile &activeLangProfile, const std::vector<LangProfile> &langProfiles)
{
    uint32_t clickedIndex = UINT32_MAX;
    if (ImGuiEx::M3::BeginMenu("###InstalledIME", ImGuiEx::M3::Spec::MenuColors::Vibrant))
    {
        uint32_t idx = 0;

        if (ImGuiEx::M3::MenuItemVibrant(DEFAULT_LANG_PROFILE.desc, activeLangProfile.guidProfile == DEFAULT_LANG_PROFILE.guidProfile))
        {
            ImeController::GetInstance()->ActivateLangProfile(DEFAULT_LANG_PROFILE.guidProfile);
        }

        for (const auto &langProfile : langProfiles)
        {
            ImGui::PushID(static_cast<int>(idx));
            const bool isSelected = IsEqualGUID(activeLangProfile.guidProfile, langProfile.guidProfile) == TRUE;
            if (ImGuiEx::M3::MenuItemVibrant(langProfile.desc, isSelected) && !isSelected)
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

auto LanguageBar::Draw(bool &pinned, const LangProfile &activeLangProfile, const std::vector<LangProfile> &langProfiles) -> bool
{
    bool openSettings = false;
    auto flags        = ImGuiEx::WindowFlags().AlwaysAutoResize().NoNav().NoDecoration();
    if (pinned)
    {
        flags = flags.NoInputs();
    }
    if (ImGuiEx::M3::BeginFloatingToolbar(LANGUAGE_BAR, nullptr, M3Spec::ToolBarColors::Vibrant, flags))
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
            openSettings = true;
        }
        ImGuiEx::M3::SetItemToolTip(Translate("Settings.Settings"));

        ImGui::SameLine();

        if (ImGuiEx::M3::Button(activeLangProfile.desc, ImGuiEx::M3::ButtonConfiguration().XSmall().Text()))
        {
            ImGui::OpenPopup("###InstalledIME");
        }
        DrawInputMethodsCombo(activeLangProfile, langProfiles);

        auto      &state          = Core::State::GetInstance();
        auto      &conversionMode = state.GetConversionMode();
        const auto cModeName      = GetConversionModeNameShort(activeLangProfile.langid, conversionMode, state.IsKeyboardOpen());
        if (!cModeName.empty())
        {
            ImGuiEx::M3::SameLine(0.F, M3Spec::StandardSmallButtonGroup::BetweenSpace);
            ImGuiEx::M3::AlignedLabel(cModeName);
        }

        ImGuiEx::M3::EndFloatingToolbar();
    }
    return openSettings;
}

} // namespace Ime
