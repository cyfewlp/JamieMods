//
// Created by jamie on 2026/2/6.
//

#include "ui/LanguageBar.h"

#include "core/State.h"
#include "i18n/TranslatorHolder.h"
#include "icons.h"
#include "ime/ImeController.h"
#include "imgui.h"
#include "imguiex/ImGuiEx.h"
#include "imguiex/imguiex_enum_wrap.h"
#include "imguiex/imguiex_m3.h"
#include "imguiex/m3/facade/button_groups.h"
#include "menu/MenuNames.h"

#include <RE/U/UIMessage.h>
#include <RE/U/UIMessageQueue.h>

namespace Ime
{

namespace
{
constexpr auto LANGUAGE_BAR = "LanguageBar";

using ColorRole = M3Spec::ColorRole;
using Spacing   = ImGuiEx::M3::Spacing;

void DrawInputMethodsCombo(const LangProfile &activeLangProfile, const std::vector<LangProfile> &langProfiles, const ImGuiEx::M3::M3Styles &m3Styles)
{
    uint32_t clickedIndex = UINT32_MAX;
    if (ImGuiEx::M3::BeginCombo("###InstalledIME", activeLangProfile.desc.c_str(), m3Styles))
    {
        uint32_t idx = 0;
        for (const auto &langProfile : langProfiles)
        {
            ImGui::PushID(static_cast<int>(idx));
            const bool isSelected = IsEqualGUID(activeLangProfile.guidProfile, langProfile.guidProfile) == TRUE;
            if (ImGuiEx::M3::MenuItem(langProfile.desc.c_str(), isSelected, m3Styles) && !isSelected)
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
        ImGuiEx::M3::EndCombo();
    }
    if (clickedIndex != UINT32_MAX)
    {
        ImeController::GetInstance()->ActivateLangProfile(langProfiles[clickedIndex].guidProfile);
    }
}

void TogglePinned(bool &pinned, bool &showing)
{
    if (pinned)
    {
        pinned = false;
    }
    else if (showing)
    {
        showing = false;
    }
    else
    {
        showing = true;
    }
}
} // namespace

auto LanguageBar::Draw(
    const bool wantToggle, const LangProfile &activeLangProfile, const std::vector<LangProfile> &langProfiles, ImGuiEx::M3::M3Styles &m3Styles
) -> bool
{
    if (wantToggle) TogglePinned(m_pinned, m_showing);

    bool openSettings = false;
    if (m_showing)
    {
        const auto styleGuard = ImGuiEx::StyleGuard()
                                    .Color<ImGuiCol_WindowBg>(m3Styles.Colors()[ColorRole::surfaceContainer])
                                    .Style<ImGuiStyleVar_WindowRounding>(m3Styles.GetPixels(M3Spec::ToolBar::rounding))
                                    .Style<ImGuiStyleVar_ItemSpacing>({m3Styles.GetPixels(M3Spec::StandardSmallButtonGroup::BetweenSpace), 0.F});

        DoDraw(openSettings, activeLangProfile, langProfiles, m3Styles);
    }
    return openSettings;
}

auto LanguageBar::DoDraw(
    bool &openSettings, const LangProfile &activeLangProfile, const std::vector<LangProfile> &langProfiles, const ImGuiEx::M3::M3Styles &m3Styles
) -> void
{
    auto flags = ImGuiEx::WindowFlags().AlwaysAutoResize().NoNav().NoDecoration();
    if (m_pinned)
    {
        flags = flags.NoInputs();
    }
    if (!ImGui::Begin(LANGUAGE_BAR, &m_showing, flags))
    {
        return;
    }

    ImGuiEx::M3::SmallIcon(ICON_MOVE, m3Styles);
    ImGui::SameLine();

    if (ImGuiEx::M3::SmallIconButton(m_pinned ? static_cast<std::string_view>(ICON_PIN_OFF) : ICON_PIN, m3Styles))
    {
        m_pinned = true;

        if (auto *const messageQueue = RE::UIMessageQueue::GetSingleton())
        {
            messageQueue->AddMessage(ToolWindowMenuName, RE::UI_MESSAGE_TYPE::kHide, nullptr);
        }
    }

    ImGui::SameLine();

    if (ImGuiEx::M3::SmallIconButton(ICON_SETTINGS, m3Styles))
    {
        openSettings = true;
    }
    ImGui::SetItemTooltip("%s", Translate("Settings.Settings"));

    ImGui::SameLine();
    DrawInputMethodsCombo(activeLangProfile, langProfiles, m3Styles);

    ImGui::SameLine();
    if (Core::State::GetInstance().Has(Core::State::IN_ALPHANUMERIC))
    {
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("ENG");
        ImGui::SameLine();
    }
    ImGui::End();
}

} // namespace Ime
