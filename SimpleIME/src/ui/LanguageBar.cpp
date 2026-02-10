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
#include "imguiex/m3/facade/button.h"
#include "menu/MenuNames.h"

#include <RE/U/UIMessage.h>
#include <RE/U/UIMessageQueue.h>

namespace Ime::LanguageBar
{

namespace
{
constexpr auto LANGUAGE_BAR = "LanguageBar";

using SurfaceToken = ImGuiEx::M3::SurfaceToken;
using ContentToken = ImGuiEx::M3::ContentToken;
using Spacing      = ImGuiEx::M3::Spacing;

void DrawInputMethodsCombo(
    const LangProfile &activeLangProfile, const std::vector<LangProfile> &langProfiles,
    const ImGuiEx::M3::M3Styles &m3Styles
)
{
    uint32_t            clickedIndex = UINT32_MAX;
    ImGuiEx::StyleGuard styleGuard;
    styleGuard.Style<ImGuiStyleVar_WindowPadding>({})
        .Style<ImGuiStyleVar_ItemSpacing>({m3Styles[Spacing::M], m3Styles[Spacing::M]})
        .Color<ImGuiCol_Text>(m3Styles.Colors()[ContentToken::onSurface])
        .Color<ImGuiCol_FrameBg>(m3Styles.Colors()[SurfaceToken::surface])
        .Color<ImGuiCol_FrameBgHovered>(m3Styles.Colors().Hovered(SurfaceToken::surface, ContentToken::onSurface))
        .Color<ImGuiCol_PopupBg>(m3Styles.Colors()[SurfaceToken::surfaceContainerLow]);

    if (ImGui::BeginCombo("###InstalledIME", activeLangProfile.desc.c_str(), ImGuiEx::ComboFlags().NoArrowButton()))
    {
        uint32_t idx = 0;
        for (const auto &langProfile : langProfiles)
        {
            ImGui::PushID(idx);
            const bool isSelected = IsEqualGUID(activeLangProfile.guidProfile, langProfile.guidProfile);
            if (ImGui::Selectable(langProfile.desc.c_str()))
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
        ImGui::EndCombo();
    }
    if (clickedIndex != UINT32_MAX)
    {
        ImeController::GetInstance()->ActivateLangProfile(langProfiles[clickedIndex].guidProfile);
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

auto DrawImpl(
    State &state, const LangProfile &activeLangProfile, const std::vector<LangProfile> &langProfiles,
    const ImGuiEx::M3::M3Styles &m3Styles
) -> State
{
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

    ImGuiEx::M3::Icon(ICON_COD_MOVE, m3Styles, ContentToken::onSurfaceVariant);
    ImGui::SameLine();

    if (ImGuiEx::M3::IconButtonSurfaceContainerVariant(IsPinned(state) ? ICON_MD_PIN : ICON_MD_PIN_OUTLINE, m3Styles))
    {
        state = static_cast<State>(state | PINNED);

        if (auto *const messageQueue = RE::UIMessageQueue::GetSingleton())
        {
            messageQueue->AddMessage(ToolWindowMenuName, RE::UI_MESSAGE_TYPE::kHide, nullptr);
        }
    }

    ImGui::SameLine();

    if (ImGuiEx::M3::IconButtonSurfaceContainerVariant(ICON_OCT_GEAR, m3Styles))
    {
        AddState(state, OPEN_SETTINGS);
    }
    ImGui::SetItemTooltip("%s", Translate("Settings.Settings"));

    ImGui::SameLine();
    DrawInputMethodsCombo(activeLangProfile, langProfiles, m3Styles);

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

} // namespace

auto Draw(
    const bool wantToggle, const LangProfile &activeLangProfile, const std::vector<LangProfile> &langProfiles,
    ImGuiEx::M3::M3Styles &m3Styles
) -> State
{
    static State state;
    if (wantToggle) TogglePinned(state);

    const auto _ = m3Styles.UseTextRole<M3Spec::SmallButton::textRole>();

    ImGuiEx::StyleGuard styleGuard;
    styleGuard.Color<ImGuiCol_WindowBg>(m3Styles.Colors()[SurfaceToken::surfaceContainer])
        .Style<ImGuiStyleVar_WindowRounding>(m3Styles.GetPixels(M3Spec::ToolBar::rounding))
        .Style<ImGuiStyleVar_FramePadding>(
            {m3Styles.GetPixels(M3Spec::ToolBar::paddingX), m3Styles.GetPixels(M3Spec::ToolBar::paddingY)}
        )
        .Style<ImGuiStyleVar_ItemSpacing>({m3Styles.GetPixels(M3Spec::ToolBar::gap), 0.f});
    DrawImpl(state, activeLangProfile, langProfiles, m3Styles);

    const auto a_copy = state;
    RemoveState(state, OPEN_SETTINGS);
    return a_copy;
}
} // namespace Ime::LanguageBar
