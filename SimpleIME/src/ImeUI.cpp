//
// Created by jamie on 25-1-21.
//

#define IMGUI_DEFINE_MATH_OPERATORS

#include "ImeUI.h"

#include "ImeWnd.hpp"
#include "common/WCharUtils.h"
#include "common/imgui/ThemesLoader.h"
#include "common/log.h"
#include "configs/CustomMessage.h"
#include "hooks/UiHooks.h"
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
        m_themesLoader.LoadThemes();
        const auto &themes = m_themesLoader.GetThemes();
        const auto  findIt = std::lower_bound(themes.begin(), themes.end(), ImGuiUtil::Theme(0, settings.theme));
        const auto  index  = std::distance(themes.begin(), findIt);
        std::expected<void, std::string> error;
        if (static_cast<size_t>(index) < themes.size())
        {
            error = m_themesLoader.UseTheme(index);
        }
        else
        {
            error = std::unexpected("Theme not found");
        }
        if (error)
        {
            settings.themeIndex = static_cast<size_t>(index);
        }
        else
        {
            ErrorNotifier::GetInstance().Warning(
                std::format("Can't find theme {}, fallback to ImGui default theme: {}", settings.theme, error.error())
            );
            if (m_themesLoader.UseTheme(0))
            {
                settings.theme      = "Dark";
                settings.themeIndex = 0;
            }
        }
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
        DrawCompWindow(settings);

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
    if (const auto &profile = installedProfiles[activatedGuid];
        ImGui::BeginCombo("###InstalledIME", profile.desc.c_str()))
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
        if (ImGui::BeginTabBar("SettingsTabBar", ImGuiTabBarFlags_None))
        {
            DrawSettingsContent(settings);
            ImGui::EndTabBar();
        }
        ImGui::PopStyleVar();
    }
    ImGui::End();
    imeManager->SyncImeStateIfDirty();
}

void ImeUI::DrawModConfig(Settings &settings)
{
    bool enableMod = settings.enableMod;
    if (ImGui::Checkbox(Translate("$Enable_Mod"), &enableMod))
    {
        ImeManagerComposer::GetInstance()->EnableMod(enableMod);
    }
    ImGui::SetItemTooltip("%s", Translate("$Enable_Mod_Tooltip"));

    ImGui::SameLine();
    const char *labelSlider = Translate("$Font_Size_Scale");
    const auto  size        = ImGui::CalcTextSize(labelSlider);
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

    if (ImGui::BeginCombo(Translate("$Themes"), settings.theme.c_str()))
    {
        size_t idx = 0;
        for (const auto &theme : m_themesLoader.GetThemes())
        {
            ImGui::PushID(idx);
            const bool isSelected = settings.themeIndex == idx;
            if (ImGui::Selectable(theme.name.c_str(), isSelected) && !isSelected)
            {
                if (m_themesLoader.UseTheme(idx))
                {
                    settings.themeIndex = idx;
                    settings.theme      = theme.name;
                }
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

void ImeUI::DrawFeatures(Settings &settings)
{
    DrawSettingsFocusManage(settings);
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

void ImeUI::DrawSettingsContent(Settings &settings)
{
    if (ImGui::BeginTabItem(Translate("$Mod_Config")))
    {
        DrawModConfig(settings);
        ImGui::EndTabItem();
    }
    if (!settings.enableMod)
    {
        return;
    }
    if (ImGui::BeginTabItem(Translate("$States")))
    {
        DrawStates();
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem(Translate("$Features")))
    {
        DrawFeatures(settings);
        ImGui::EndTabItem();
    }
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

void ImeUI::DrawStates() const
{
    ImGui::SeparatorText(Translate("$States"));

    constexpr auto getStateIcon = [](const bool state) {
        return state ? EMOJI_YES : EMOJI_NO;
    };

    ImGui::Text("%s: %s", Translate("$Ime_Enabled"), getStateIcon(State::GetInstance().NotHas(State::IME_DISABLED)));
    ImGui::SetItemTooltip("%s", Translate("$Ime_Enabled_Tooltip"));
    ImGui::Text("%s: %s", Translate("$Ime_Focus"), getStateIcon(m_pImeWnd->IsFocused()));
    ImGui::SetItemTooltip("%s", Translate("$Ime_Focus_Tooltip"));

    float spacing = ImGui::GetFontSize() * 2;
    ImGui::SameLine(0, spacing);
    if (ImGui::Button(Translate("$Force_Focus_Ime")))
    {
        ImeManagerComposer::GetInstance()->ForceFocusIme();
    }
    ImGui::Text(
        "%s: %s",
        Translate("$Message_Filter_Enabled"),
        getStateIcon(Hooks::UiHooks::GetInstance()->IsEnableMessageFilter())
    );
    ImGui::SetItemTooltip("%s", Translate("$Message_Filter_Enabled_Tooltip"));
    ImGui::SameLine(0, spacing);
    if (ImGui::Button(Translate("$Message_Filter_Close")))
    {
        Hooks::UiHooks::GetInstance()->EnableMessageFilter(false);
    }
    ImGui::SetItemTooltip("%s", Translate("$Message_Filter_Close_Tooltip"));
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

void ImeUI::DrawSettingsFocusManage(Settings &settings) const
{
    // Focus Manage widget
    auto *imeManager = ImeManagerComposer::GetInstance();

    ImGui::SeparatorText(Translate("$Focus_Manage"));

    bool pressed =
        RadioButton(Translate("$Focus_Manage_Permanent"), &settings.focusType, Settings::FocusType::Permanent);
    ImGui::SetItemTooltip("%s", Translate("$Focus_Manage_Permanent_Tooltip"));
    ImGui::SameLine();
    pressed |= RadioButton(Translate("$Focus_Manage_Temporary"), &settings.focusType, Settings::FocusType::Temporary);
    ImGui::SetItemTooltip("%s", Translate("$Focus_Manage_Temporary_Tooltip"));

    if (pressed)
    {
        imeManager->PopAndPushType(settings.focusType);
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

void ImeUI::DrawCompWindow(const Settings &settings) const
{
    static float CursorAnim = 0.f;

    CursorAnim += ImGui::GetIO().DeltaTime;

    const ImVec4 highLightText = ImGui::GetStyle().Colors[ImGuiCol_TextLink];
    auto        &textEditor    = m_pTextService->GetTextEditor();

    const auto &editorText = textEditor.GetText();
    LONG        acpStart   = 0;
    LONG        acpEnd     = 0;
    textEditor.GetSelection(&acpStart, &acpEnd);

    bool success = true;
    // we assume start always == end
    if (acpStart != acpEnd)
    {
        std::string text;
        success = WCharUtils::ToString(editorText.c_str(), acpStart, text);
        if (success)
        {
            ImGui::TextColored(highLightText, "%s", text.c_str());
        }
    }
    else if (static_cast<size_t>(acpStart) <= editorText.size())
    {
        ImGui::PushStyleVarX(ImGuiStyleVar_ItemSpacing, 0);
        if (acpStart > 0)
        {
            std::string startToCaret;
            success = WCharUtils::ToString(editorText.c_str(), acpStart, startToCaret);
            if (success)
            {
                ImGui::TextColored(highLightText, "%s", startToCaret.c_str());
            }
        }

        // caret
        ImGui::SameLine();
        if (fmodf(CursorAnim, 1.2f) <= 0.8f)
        {
            ImDrawList *drawList        = ImGui::GetWindowDrawList();
            ImVec2      cursorScreenPos = ImGui::GetCursorScreenPos();
            ImVec2      min(cursorScreenPos.x, cursorScreenPos.y + 0.5f);
            drawList->AddLine(
                min,
                ImVec2(min.x, cursorScreenPos.y + ImGui::GetFontSize() - 1.5f),
                ImGui::GetColorU32(ImGuiCol_InputTextCursor),
                1.0f * settings.dpiScale
            );
        }

        success = success && static_cast<size_t>(acpStart) < editorText.size();
        if (success)
        {
            std::string caretToEnd;
            success = WCharUtils::ToString(editorText.c_str() + acpStart, editorText.size() - acpStart, caretToEnd);
            if (success)
            {
                ImGui::TextColored(highLightText, "%s", caretToEnd.c_str());
            }
        }
        if (!success)
        {
            ImGui::NewLine();
        }
        ImGui::PopStyleVar();
    }
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