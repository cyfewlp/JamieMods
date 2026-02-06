//
// Created by jamie on 2026/2/6.
//

#include "ui/LanguageBar.h"

#include "common/imgui/ImGuiEx.h"
#include "core/State.h"
#include "i18n/TranslatorHolder.h"
#include "icons.h"
#include "ime/ImeController.h"
#include "imgui.h"
#include "menu/MenuNames.h"

#include <RE/U/UIMessage.h>
#include <RE/U/UIMessageQueue.h>
#include <cguid.h>

namespace Ime::LanguageBar
{

namespace
{
constexpr auto LANGUAGE_BAR = "LanguageBar";

void DrawInputMethodsCombo(const GUID &activeLangGuid, const std::unordered_map<GUID, LangProfile> &langProfiles)
{
    GUID currentLangGuid = activeLangGuid;
    if (!langProfiles.contains(activeLangGuid))
    {
        currentLangGuid = GUID_NULL;
    }
    const GUID *clickedLang = nullptr;
    if (const auto &profile = langProfiles.at(currentLangGuid);
        ImGui::BeginCombo("###InstalledIME", profile.desc.c_str()))
    {
        int32_t idx = 0;
        for (const std::pair<GUID, LangProfile> pair : langProfiles)
        {
            ImGui::PushID(idx);
            const auto &langProfile = pair.second;
            const bool  isSelected  = langProfile.guidProfile == currentLangGuid;
            if (ImGui::Selectable(langProfile.desc.c_str()))
            {
                clickedLang = &pair.first;
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
    if (clickedLang != nullptr)
    {
        ImeController::GetInstance()->ActivateLangProfile(clickedLang);
    }
}

void AddState(State &state, const State newState)
{
    state = static_cast<State>(state | newState);
}

void RemoveState(State &state, const State newState)
{
    state = static_cast<State>(state & ~newState);
}

void TogglePinned(State &state)
{
    if (IsPinned(state))
    {
        RemoveState(state, PINNED);
    }
    else if (IsShowing(state))
    {
        RemoveState(state, SHOWING);
    }
    else
    {
        AddState(state, SHOWING);
    }
}

void SetShowing(State &state, bool showing)
{
    if (showing)
    {
        AddState(state, SHOWING);
    }
    else
    {
        RemoveState(state, SHOWING);
    }
}

} // namespace

auto Draw(const bool wantToggle, const GUID &activeLangGuid, const std::unordered_map<GUID, LangProfile> &langProfiles)
    -> State
{
    static State state;
    if (wantToggle) TogglePinned(state);

    if (!IsShowing(state)) return state;

    auto flags = ImGuiEx::WindowFlags().AlwaysAutoResize().NoNav().NoDecoration();
    if (IsPinned(state))
    {
        flags = flags.NoInputs();
    }
    bool showing = IsShowing(state);
    bool visible = ImGui::Begin(LANGUAGE_BAR, &showing, flags);
    SetShowing(state, showing);
    if (!visible) return state;

    ImGui::AlignTextToFramePadding();
    ImGui::Text(ICON_COD_MOVE);
    ImGui::SameLine();

    if (ImGui::Button(IsPinned(state) ? ICON_MD_PIN : ICON_MD_PIN_OUTLINE))
    {
        state = static_cast<State>(state | PINNED);

        if (auto *const messageQueue = RE::UIMessageQueue::GetSingleton())
        {
            messageQueue->AddMessage(ToolWindowMenuName, RE::UI_MESSAGE_TYPE::kHide, nullptr);
        }
    }

    ImGui::SameLine();
    if (ImGui::Button(ICON_OCT_GEAR))
    {
        AddState(state, OPEN_SETTINGS);
    }
    ImGui::SetItemTooltip("%s", Translate("Settings.Settings"));

    ImGui::SameLine();
    DrawInputMethodsCombo(activeLangGuid, langProfiles);

    ImGui::SameLine();
    if (Core::State::GetInstance().Has(Core::State::IN_ALPHANUMERIC))
    {
        ImGui::AlignTextToFramePadding();
        ImGui::Text("ENG");
        ImGui::SameLine();
    }
    ImGui::End();
    return state;
}
} // namespace Ime::LanguageBar
