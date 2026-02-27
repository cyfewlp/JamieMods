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

void DrawInputMethodsCombo(const LangProfile &activeLangProfile, const std::vector<LangProfile> &langProfiles)
{
    uint32_t clickedIndex = UINT32_MAX;
    if (ImGuiEx::M3::BeginMenu("###InstalledIME", ImGuiEx::M3::Spec::MenuColors::Vibrant))
    {
        uint32_t idx = 0;
        for (const auto &langProfile : langProfiles)
        {
            ImGui::PushID(static_cast<int>(idx));
            const bool isSelected = IsEqualGUID(activeLangProfile.guidProfile, langProfile.guidProfile) == TRUE;
            if (ImGuiEx::M3::MenuItemVibrant(langProfile.desc.c_str(), isSelected) && !isSelected)
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

auto LanguageBar::Draw(const bool wantToggle, const LangProfile &activeLangProfile, const std::vector<LangProfile> &langProfiles) -> bool
{
    if (wantToggle) TogglePinned(m_pinned, m_showing);

    bool openSettings = false;
    if (m_showing)
    {
        DoDraw(openSettings, activeLangProfile, langProfiles);
    }
    return openSettings;
}

auto LanguageBar::DoDraw(bool &openSettings, const LangProfile &activeLangProfile, const std::vector<LangProfile> &langProfiles) -> void
{
    auto flags = ImGuiEx::WindowFlags().AlwaysAutoResize().NoNav().NoDecoration();
    if (m_pinned)
    {
        flags = flags.NoInputs();
    }
    if (ImGuiEx::M3::BeginFloatingToolbar(LANGUAGE_BAR, &m_showing, M3Spec::ToolBarColors::Vibrant, flags))
    {
        ImGuiEx::M3::SmallIcon(ICON_MOVE);
        ImGui::SameLine();

        constexpr auto iconButtonColors = ImGuiEx::M3::Spec::IconButtonColors::Standard;
        if (ImGuiEx::M3::SmallIconButton(m_pinned ? static_cast<std::string_view>(ICON_PIN_OFF) : ICON_PIN, iconButtonColors))
        {
            m_pinned = true;

            if (auto *const messageQueue = RE::UIMessageQueue::GetSingleton())
            {
                messageQueue->AddMessage(ToolWindowMenuName, RE::UI_MESSAGE_TYPE::kHide, nullptr);
            }
        }

        ImGui::SameLine();

        if (ImGuiEx::M3::SmallIconButton(ICON_SETTINGS, iconButtonColors))
        {
            openSettings = true;
        }
        ImGui::SetItemTooltip("%s", Translate("Settings.Settings"));

        ImGui::SameLine();

        if (ImGuiEx::M3::SmallButton(activeLangProfile.desc.c_str(), ""))
        {
            ImGui::OpenPopup("###InstalledIME");
        }
        DrawInputMethodsCombo(activeLangProfile, langProfiles);

        ImGui::SameLine();
        if (Core::State::GetInstance().Has(Core::State::IN_ALPHANUMERIC))
        {
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("ENG");
            ImGui::SameLine();
        }

        ImGuiEx::M3::EndFloatingToolbar();
    }
}

} // namespace Ime
