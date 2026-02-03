//
// Created by jamie on 25-1-21.
//

#define IMGUI_DEFINE_MATH_OPERATORS

#include "ImeUI.h"

#include "ImeWnd.hpp"
#include "common/imgui/ImGuiEx.h"
#include "common/imgui/Material3.h"
#include "common/imgui/imgui_m3_ex.h"
#include "common/log.h"
#include "configs/CustomMessage.h"
#include "core/State.h"
#include "i18n/TranslationLoader.h"
#include "i18n/TranslatorHolder.h"
#include "icons.h"
#include "ime/ImeController.h"
#include "imgui.h"
#include "menu/MenuNames.h"
#include "tsf/LangProfileUtil.h"
#include "ui/Settings.h"

#include <RE/U/UIMessage.h>
#include <RE/U/UIMessageQueue.h>
#include <cguid.h>
#include <cstdint>
#include <format>
#include <minwindef.h>
#include <string>
#include <utility>
#include <winnt.h>

#pragma comment(lib, "dwrite.lib")

namespace Ime
{

constexpr auto TRANSLATE_FILES_DIR = "Data/interface/SimpleIME";

ImeUI::~ImeUI()
{
    m_langProfileUtil->Release();
}

bool ImeUI::Initialize(LangProfileUtil *pLangProfileUtil)
{
    logger::debug("Initializing ImeUI...");
    m_langProfileUtil = pLangProfileUtil;
    m_langProfileUtil->AddRef();
    if (!m_langProfileUtil->LoadAllLangProfiles())
    {
        logger::error("Failed load lang profiles");
        return false;
    }
    if (!m_langProfileUtil->LoadActiveIme())
    {
        logger::error("Failed load active ime");
        return false;
    }
    m_fontBuilder.Initialize();
    return true;
}

void ImeUI::ApplySettings(Settings::Appearance &appearance, ImGuiEx::M3::M3Styles &m3Styles)
{
    m_panelAppearance.ApplySettings(appearance, m3Styles);
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

void ImeUI::DrawToolWindow(Settings &settings, ImGuiEx::M3::M3Styles &m3Styles)
{
    if (!m_fShowToolWindow)
    {
        return;
    }

    auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration;
    if (m_fPinToolWindow)
    {
        flags |= ImGuiWindowFlags_NoInputs;
    }
    ImGui::Begin(TOOL_WINDOW_NAME.data(), &m_fShowToolWindow, flags);

    DrawSettings(settings, m3Styles);

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
    ImGui::SetItemTooltip("%s", Translate("Settings.Settings"));

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

void ImeUI::DrawSettings(Settings &settings, ImGuiEx::M3::M3Styles &m3Styles)
{
    if (!settings.appearance.showSettings)
    {
        return;
    }
    auto      *imeManager = ImeController::GetInstance();
    const auto windowName = std::format("{}###SettingsWindow", Translate("Settings.Settings"));

    ImGuiEx::StyleGuard styleGuard;
    styleGuard.Color_Text(m3Styles.Colors()[ImGuiEx::M3::ContentToken::onSurface]).Style_WindowPadding({});
    if (ImGui::Begin(windowName.c_str(), &settings.appearance.showSettings))
    {
        enum class Menu : int8_t
        {
            Appearance,
            FontBuilder,
            Behaviour
        };
        static auto currentMenu = std::pair{Menu::Appearance, true};

        // Sidebar
        {
            ImGuiEx::StyleGuard styleGuard1;
            styleGuard1.Color_Text(m3Styles.Colors()[ImGuiEx::M3::ContentToken::onSurfaceVariant])
                .Color_ChildBg(m3Styles.Colors()[ImGuiEx::M3::SurfaceToken::surfaceContainer])
                .Color_FrameBg(m3Styles.Colors()[ImGuiEx::M3::SurfaceToken::surfaceContainer])
                .Color_FrameBgActive(m3Styles.Colors()[ImGuiEx::M3::SurfaceToken::secondary])
                .Color_FrameBgHovered(m3Styles.Colors()[ImGuiEx::M3::SurfaceToken::secondaryContainer]);

            if (ImGui::BeginChild(
                    "Sidebar",
                    {m3Styles.GetSize(ImGuiEx::M3::ComponentSize::NAV_RAIL_WIDTH), -FLT_MIN},
                    ImGuiEx::ChildFlags().Borders()
                ))
            {
                ImGuiEx::M3::DrawNavMenu(ICON_MD_MENU, m3Styles);
                if (ImGuiEx::M3::DrawNavItem(
                        Translate("Settings.Sidebar.Appearance"),
                        currentMenu.first == Menu::Appearance,
                        ICON_MD_PALETTE,
                        m3Styles
                    ))
                {
                    currentMenu = {Menu::Appearance, true};
                }
                if (ImGuiEx::M3::DrawNavItem(
                        Translate("Settings.Sidebar.FontBuilder"),
                        currentMenu.first == Menu::FontBuilder,
                        ICON_FA_WRENCH,
                        m3Styles
                    ))
                {
                    currentMenu = {Menu::FontBuilder, true};
                }
                if (ImGuiEx::M3::DrawNavItem(
                        Translate("Settings.Sidebar.Behaviour"),
                        currentMenu.first == Menu::Behaviour,
                        ICON_OCT_GEAR,
                        m3Styles
                    ))
                {
                    currentMenu = {Menu::Behaviour, true};
                }
            }
            ImGui::EndChild();
        }

        ImGui::SameLine(0, 0);

        ImGui::BeginGroup();
        switch (currentMenu.first)
        {
            case Menu::Appearance:
                DrawMenuAppearance(settings, m3Styles);
                break;
            case Menu::FontBuilder:
                DrawMenuFontBuilder(settings, m3Styles);
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

void ImeUI::DrawMenuAppearance(Settings &settings, ImGuiEx::M3::M3Styles &m3Styles)
{
    m_panelAppearance.Draw(settings, m3Styles);

    // sync theme config
    settings.appearance.themeSourceColor = m3Styles.Colors().SourceColor();
    settings.appearance.themeDarkMode    = m3Styles.Colors().DarkMode();
}

void ImeUI::DrawMenuFontBuilder(Settings &settings, const ImGuiEx::M3::M3Styles &m3Styles)
{
    m_fontBuilderView.Draw(m_fontBuilder, settings, m3Styles);
}

void ImeUI::DrawMenuBehaviour(Settings &settings) const
{
    bool enableMod = settings.enableMod;
    if (ImGui::Checkbox(Translate("Settings.Behaviour.EnableMod"), &enableMod))
    {
        ImeController::GetInstance()->EnableMod(enableMod);
    }
    ImGui::SetItemTooltip("%s", Translate("Settings.Behaviour.EnableModToolTip"));
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
    ImGui::SetItemTooltip("%s", Translate("Settings.Behaviour.EnableUnicodePasteTooltip"));

    ImGui::SameLine();
    if (ImGui::Checkbox(Translate("Settings.Behaviour.KeepImeOpen"), &settings.input.keepImeOpen))
    {
        ImeController::GetInstance()->MarkDirty();
    }
    ImGui::SetItemTooltip("%s", Translate("Settings.Behaviour.KeepImeOpenTooltip"));
}

void ImeUI::DrawStates() const
{
    ImGui::SeparatorText(Translate("Settings.Behaviour.States"));

    constexpr auto STATE_ACTIVE_COLOR = ImVec4(0.35F, 0.75F, 1.0F, 1.0F);
    const auto    &state              = State::GetInstance();
    ImGui::AlignTextToFramePadding();
    ImGui::TextColored(
        state.NotHas(State::IME_DISABLED) ? STATE_ACTIVE_COLOR : inactiveColor, "[ %s ]", ICON_FA_KEYBOARD
    );
    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", Translate("Settings.Behaviour.ImeEnabled"));
    ImGui::SetItemTooltip("%s", Translate("Settings.Behaviour.ImeEnabledTooltip"));

    ImGui::SameLine();

    ImGui::AlignTextToFramePadding();
    ImGui::TextColored(m_pImeWnd->IsFocused() ? STATE_ACTIVE_COLOR : inactiveColor, "[ %s ]", ICON_FA_CROSSHAIR);
    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", Translate("Settings.Behaviour.Focus"));
    ImGui::SetItemTooltip("%s", Translate("Settings.Behaviour.FocusTooltip"));

    ImGui::SameLine(0, ImGui::GetFontSize());
    ImGui::TextDisabled("|");
    ImGui::SameLine(0, ImGui::GetFontSize());
    if (ImGui::Button(Translate("Settings.Behaviour.ForceFocusIme")))
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

    ImGui::SeparatorText(Translate("Settings.Behaviour.ImePos.Policy"));

    RadioButton(
        Translate("Settings.Behaviour.ImePos.UpdateByCursor"), &settings.input.posUpdatePolicy, Policy::BASED_ON_CURSOR
    );
    ImGui::SetItemTooltip("%s", Translate("Settings.Behaviour.ImePos.UpdateByCursorTooltip"));

    ImGui::SameLine();
    RadioButton(
        Translate("Settings.Behaviour.ImePos.UpdateByCaret"), &settings.input.posUpdatePolicy, Policy::BASED_ON_CARET
    );
    ImGui::SetItemTooltip("%s", Translate("Settings.Behaviour.ImePos.UpdateByCaretTooltip"));

    ImGui::SameLine();
    RadioButton(Translate("Settings.Behaviour.ImePos.UpdateByNone"), &settings.input.posUpdatePolicy, Policy::NONE);
    ImGui::SetItemTooltip("%s", Translate("Settings.Behaviour.ImePos.UpdateByNoneTooltip"));
}

} // namespace Ime
