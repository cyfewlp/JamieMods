//
// Created by jamie on 25-1-21.
//

#define IMGUI_DEFINE_MATH_OPERATORS

#include "ImeUI.h"

#include "ImGuiThemeLoader.h"
#include "ImeWnd.hpp"
#include "common/WCharUtils.h"
#include "common/log.h"
#include "configs/CustomMessage.h"
#include "ime/ImeManagerComposer.h"
#include "imgui.h"
#include "tsf/LangProfileUtil.h"
#include "utils/FocusGFxCharacterInfo.h"

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{

static constexpr ImVec4 RED_COLOR = {1.0F, 0.0F, 0.0F, 1.0F};
constexpr auto         *EMOJI_YES = "✅";
constexpr auto         *EMOJI_NO  = "❌";

ImeUI::~ImeUI()
{
    if (m_pTextService != nullptr)
    {
        m_langProfileUtil->Release();
    }
}

bool ImeUI::Initialize(LangProfileUtil *pLangProfileUtil)
{
    log_debug("Initializing ImeUI...");
    m_langProfileUtil = pLangProfileUtil;
    m_langProfileUtil->AddRef();
    if (!m_langProfileUtil->LoadAllLangProfiles())
    {
        log_error("Failed load lang profiles");
        return false;
    }
    if (!m_langProfileUtil->LoadActiveIme())
    {
        log_error("Failed load active ime");
        return false;
    }
    if (!m_translation.GetTranslateLanguages(m_uiConfig.TranslationDir(), m_translateLanguages))
    {
        log_warn("Failed load translation languages.");
    }
    return true;
}

void ImeUI::ApplyUiSettings(Settings &settings)
{
    ImGui::GetIO().FontGlobalScale = settings.fontSizeScale;

    { // Apply language config
        if (const auto langIt = std::ranges::find(m_translateLanguages, settings.language);
            langIt == m_translateLanguages.end())
        {
            settings.language = "english";
        }
        m_translation.UseLanguage(settings.language.c_str());
    }

    { // Apply theme config
        const auto findIt = std::ranges::find(m_themeNames, settings.theme);
        if (findIt != m_themeNames.end())
        {
            m_uiThemeLoader.LoadTheme(settings.theme, ImGui::GetStyle());
        }
        else
        {
            settings.theme = "";
            ErrorNotifier::GetInstance().Warning(
                std::format("Can't find theme {}, fallback to ImGui default theme.", settings.theme)
            );
            ImGui::StyleColorsDark();
        }
    }
}

void ImeUI::SetTheme() // TODO
{
    if (m_uiConfig.UseClassicTheme())
    {
        ImGui::StyleColorsDark();
        return;
    }

    if (!m_uiThemeLoader.GetAllThemeNames(m_uiConfig.ThemeDirectory(), m_themeNames))
    {
        log_warn("Failed get theme names, fallback to ImGui default theme.");
        ImGui::StyleColorsDark();
    }
}

void ImeUI::Draw(const Settings &settings)
{
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize;
    windowFlags |= ImGuiWindowFlags_NoDecoration;
    windowFlags |= ImGuiWindowFlags_AlwaysAutoResize;
    static bool prevFrameIsShowing = false;
    const auto &state              = State::GetInstance();
    if (state.Has(State::IME_DISABLED) || state.NotHas(State::IN_CAND_CHOOSING, State::IN_COMPOSING))
    {
        prevFrameIsShowing = false;
        return;
    }
    if (!prevFrameIsShowing)
    {
        CalculateWindowSize();
        if (ImVec2 windowPos; UpdateImeWindowPos(settings, windowPos))
        {
            ImGui::SetNextWindowPos(windowPos);
        }
    }
    prevFrameIsShowing = true;

    if (ImGui::Begin(SKSE::PluginDeclaration::GetSingleton()->GetName().data(), nullptr, windowFlags))
    {
        m_imeWindowSize = ImGui::GetContentRegionAvail();
        RenderCompWindow();

        ImGui::Separator();
        // render ime status window: language,
        if (state.Has(State::IN_CAND_CHOOSING))
        {
            DrawCandidateWindows();
        }
    }
    ImGui::End();
}

auto ImeUI::UpdateImeWindowPos(const Settings &settings, ImVec2 &windowPos) const -> bool
{
    switch (settings.windowPosUpdatePolicy)
    {
        case Settings::WindowPosUpdatePolicy::BASED_ON_CURSOR:
            if (const auto *cursor = RE::MenuCursor::GetSingleton(); cursor != nullptr)
            {
                windowPos.x = cursor->cursorPosX;
                windowPos.y = cursor->cursorPosY;
            }
            return true;
        case Settings::WindowPosUpdatePolicy::BASED_ON_CARET: {
            FocusGFxCharacterInfo::GetInstance().UpdateCaretCharBoundaries();
            return UpdateImeWindowPosByCaret(windowPos);
        }
        default:;
    }
    return false;
}

auto ImeUI::UpdateImeWindowPosByCaret(ImVec2 &windowPos) const -> bool
{
    const auto &instance = FocusGFxCharacterInfo::GetInstance();
    if (m_imeWindowSize.x == 0 || m_imeWindowSize.y == 0)
    {
        return false;
    }
    const auto &charBoundaries       = instance.CharBoundaries();
    auto [screenWidth, screenHeight] = RE::BSGraphics::Renderer::GetScreenSize();

    windowPos.x = charBoundaries.left;
    windowPos.y = charBoundaries.bottom;

    if (windowPos.x + m_imeWindowSize.x > static_cast<float>(screenWidth))
    {
        windowPos.y = screenWidth - m_imeWindowSize.x;
    }
    if (windowPos.y + m_imeWindowSize.y > static_cast<float>(screenHeight))
    {
        windowPos.y = screenHeight - m_imeWindowSize.y;
    }
    return true;
}

void ImeUI::CalculateWindowSize()
{
    const auto &candidateUi = m_pTextService->GetCandidateUi();
    const auto &style       = ImGui::GetStyle();
    m_imeWindowSize.x       = 0;
    m_imeWindowSize.y       = 0;
    if (const auto candidateList = candidateUi.CandidateList(); candidateList.size() > 0)
    {
        for (const auto &candidate : candidateList)
        {
            auto textSize = ImGui::CalcTextSize(candidate.c_str());
            m_imeWindowSize.x += textSize.x;
            m_imeWindowSize.y = std::max(m_imeWindowSize.y, textSize.y);
        }
    }
    {
        const auto &editorText = m_pTextService->GetTextEditor().GetText();
        const auto  str        = WCharUtils::ToString(editorText);
        const auto  textSize   = ImGui::CalcTextSize(str.c_str());
        m_imeWindowSize.x      = std::max(m_imeWindowSize.x, textSize.x);
        m_imeWindowSize.y      = std::max(m_imeWindowSize.y, textSize.y);
    }
    m_imeWindowSize.x += style.WindowPadding.x * 2;
    m_imeWindowSize.y += m_imeWindowSize.y + style.WindowPadding.y * 2 + style.ItemSpacing.y;
}

void ImeUI::DrawInputMethodsCombo() const
{
    auto &activatedGuid     = m_langProfileUtil->GetActivatedLangProfile();
    auto &installedProfiles = m_langProfileUtil->GetLangProfiles();
    if (!installedProfiles.contains(activatedGuid))
    {
        activatedGuid = GUID_NULL;
    }
    auto &profile = installedProfiles[activatedGuid];
    if (ImGui::BeginCombo("###InstalledIME", profile.desc.c_str()))
    {
        uint32_t idx = 0;
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
        m_fPinToolWindow               = false;
        ImGui::GetIO().MouseDrawCursor = true;
        ImGui::SetWindowFocus(TOOL_WINDOW_NAME.data());
    }
    else
    {
        m_fShowToolWindow              = !m_fShowToolWindow;
        ImGui::GetIO().MouseDrawCursor = m_fShowToolWindow;
        if (m_fShowToolWindow)
        {
            ImGui::SetWindowFocus(TOOL_WINDOW_NAME.data());
        }
    }
}

void ImeUI::RenderToolWindow(Settings &settings)
{
    if (!m_fShowToolWindow)
    {
        return;
    }

    m_translation.UseSection("Tool Window");
    auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration;
    if (m_fPinToolWindow)
    {
        flags |= ImGuiWindowFlags_NoInputs;
    }
    ImGui::Begin(TOOL_WINDOW_NAME.data(), &m_fShowToolWindow, flags);

    DrawSettings(settings);

    m_translation.UseSection("Tool Window");

    ImGui::Text("Drag");
    ImGui::SameLine();

    if (!m_errorMessages.empty())
    {
        for (const auto &errorMessage : m_errorMessages)
        {
            ImGui::TextColored(RED_COLOR, "%s", errorMessage.c_str());
        }
    }

    if (ImGui::Button("\xf0\x9f\x93\x8c")) // pin
    {
        m_fPinToolWindow               = true;
        ImGui::GetIO().MouseDrawCursor = false;
    }

    ImGui::SameLine();
    ImGui::Checkbox(Translate("$Settings"), &settings.showSettings);

    ImGui::SameLine();
    DrawInputMethodsCombo();

    ImGui::SameLine();
    if (State::GetInstance().Has(State::IN_ALPHANUMERIC))
    {
        ImGui::Text("ENG");
        ImGui::SameLine();
    }
    ImGui::End();
}

void ImeUI::DrawSettings(Settings &settings)
{
    if (!settings.showSettings)
    {
        return;
    }
    auto      *imeManager = ImeManagerComposer::GetInstance();
    const auto windowName = std::format("{}###SettingsWindow", m_translation.Get("$Settings"));
    if (ImGui::Begin(windowName.c_str(), &settings.showSettings, ImGuiWindowFlags_NoNav))
    {
        m_translation.UseSection("Settings");
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10, 4));
        DrawSettingsContent(settings);
        ImGui::PopStyleVar();

        if (DrawCombo(Translate("$Themes"), m_themeNames, settings.theme))
        {
            m_uiThemeLoader.LoadTheme(settings.theme, ImGui::GetStyle());
        }
    }
    ImGui::End();
    imeManager->SyncImeStateIfDirty();
}

void ImeUI::DrawSettingsContent(Settings &settings)
{
    if (ImGui::Checkbox(Translate("$Enable_Mod"), &settings.enableMod))
    {
        ImeManagerComposer::GetInstance()->EnableMod(settings.enableMod);
    }
    ImGui::SetItemTooltip("%s", Translate("$Enable_Mod_Tooltip"));

    ImGui::SameLine();
    const char *labelSlider = Translate("$Font_Size_Scale");
    auto        size        = ImGui::CalcTextSize(labelSlider);
    ImGui::SetNextItemWidth(-size.x);
    ImGui::DragFloat(
        Translate("$Font_Size_Scale"),
        &ImGui::GetIO().FontGlobalScale,
        0.05,
        SettingsConfig::MIN_FONT_SIZE_SCALE,
        SettingsConfig::MAX_FONT_SIZE_SCALE,
        "%.3f",
        ImGuiSliderFlags_NoInput
    );

    if (DrawCombo(Translate("$Languages"), m_translateLanguages, settings.language))
    {
        m_translation.UseLanguage(settings.language.c_str());
    }

    if (!settings.enableMod)
    {
        return;
    }
    DrawSettingsState();

    ImGui::SeparatorText(Translate("$Features"));

    DrawSettingsFocusManage();
    DrawWindowPosUpdatePolicy(settings);

    ImGui::Checkbox(Translate("$Enable_Unicode_Paste"), &settings.enableUnicodePaste);
    ImGui::SetItemTooltip("%s", Translate("$Enable_Unicode_Paste_Tooltip"));

    ImGui::SameLine();
    if (ImGui::Checkbox(Translate("$Keep_Ime_Open"), &settings.keepImeOpen))
    {
        ImeManagerComposer::GetInstance()->MarkDirty();
    }
    ImGui::SetItemTooltip("%s", Translate("$Keep_Ime_Open_Tooltip"));
}

auto ImeUI::DrawCombo(const char *label, const std::vector<std::string> &values, std::string &selected) -> bool
{
    bool clicked = false;
    if (ImGui::BeginCombo(label, selected.c_str()))
    {
        uint32_t idx = 0;
        for (const auto &language : values)
        {
            ImGui::PushID(idx);
            const bool isSelected = selected == language;
            if (ImGui::Selectable(language.c_str(), isSelected) && !isSelected)
            {
                selected = language;
                clicked  = true;
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
    return clicked;
}

void ImeUI::DrawSettingsState() const
{
    ImGui::SeparatorText(Translate("$States"));

    ImGui::Text(
        "%s: %s", Translate("$Ime_Enabled"), State::GetInstance().NotHas(State::IME_DISABLED) ? EMOJI_YES : EMOJI_NO
    );
    ImGui::SetItemTooltip("%s", Translate("$Ime_Enabled_Tooltip"));

    ImGui::Text("%s: %s", Translate("$Ime_Focus"), m_pImeWnd->IsFocused() ? EMOJI_YES : EMOJI_NO);
    ImGui::SetItemTooltip("%s", Translate("$Ime_Focus_Tooltip"));

    ImGui::SameLine();
    if (ImGui::Button(Translate("$Force_Focus_Ime")))
    {
        ImeManagerComposer::GetInstance()->ForceFocusIme();
    }
}

template <typename T>
constexpr auto RadioButton(const char *label, T *pValue, T value) -> bool
{
    bool pressed = ImGui::RadioButton(label, *pValue == value);
    if (pressed)
    {
        *pValue = value;
    }
    return pressed;
}

void ImeUI::DrawSettingsFocusManage() const
{
    // Focus Manage widget
    auto *imeManager = ImeManagerComposer::GetInstance();

    ImGui::SeparatorText(Translate("$Focus_Manage"));

    auto focusType = imeManager->GetFocusManageType();

    bool pressed = RadioButton(Translate("$Focus_Manage_Permanent"), &focusType, Settings::FocusType::Permanent);
    ImGui::SetItemTooltip("%s", Translate("$Focus_Manage_Permanent_Tooltip"));
    ImGui::SameLine();
    pressed |= RadioButton(Translate("$Focus_Manage_Temporary"), &focusType, Settings::FocusType::Temporary);
    ImGui::SetItemTooltip("%s", Translate("$Focus_Manage_Temporary_Tooltip"));

    if (pressed)
    {
        imeManager->PopAndPushType(focusType);
    }
}

void ImeUI::DrawWindowPosUpdatePolicy(Settings &settings)
{
    using Policy = Settings::WindowPosUpdatePolicy;
    m_translation.UseSection("Ime Window Pos");
    {
        ImGui::SeparatorText(Translate("$Policy"));

        RadioButton(Translate("$Update_By_Cursor"), &settings.windowPosUpdatePolicy, Policy::BASED_ON_CURSOR);
        ImGui::SetItemTooltip("%s", Translate("$Update_By_Cursor_Tooltip"));

        ImGui::SameLine();
        RadioButton(Translate("$Update_By_Caret"), &settings.windowPosUpdatePolicy, Policy::BASED_ON_CARET);
        ImGui::SetItemTooltip("%s", Translate("$Update_By_Caret_Tooltip"));

        ImGui::SameLine();
        RadioButton(Translate("$Update_By_None"), &settings.windowPosUpdatePolicy, Policy::NONE);
        ImGui::SetItemTooltip("%s", Translate("$Update_By_None_Tooltip"));
    }
    m_translation.UseSection("Settings");
}

void ImeUI::RenderCompWindow() const
{
    const ImVec4 highLightText = ImGui::GetStyle().Colors[ImGuiCol_TextLink];
    const auto  &editorText    = m_pTextService->GetTextEditor().GetText();
    const auto   str           = WCharUtils::ToString(editorText);
    ImGui::TextColored(highLightText, "%s", str.c_str());
}

void ImeUI::DrawCandidateWindows() const
{
    const auto &candidateUi = m_pTextService->GetCandidateUi();
    if (const auto candidateList = candidateUi.CandidateList(); candidateList.size() > 0)
    {
        DWORD index = 0;
        for (const auto &candidate : candidateList)
        {
            if (index == candidateUi.Selection())
            {
                ImVec4 highLightText = ImGui::GetStyle().Colors[ImGuiCol_TextLink];
                ImGui::TextColored(highLightText, "%s", candidate.c_str());
            }
            else
            {
                ImGui::Text("%s", candidate.c_str());
            }
            ImGui::SameLine();
            index++;
        }
    }
}

inline auto ImeUI::Translate(const char *label) const -> const char *
{
    return m_translation.Get(label);
}

} // namespace  SimpleIME
} // namespace LIBC_NAMESPACE_DECL