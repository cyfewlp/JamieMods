//
// Created by jamie on 25-1-21.
//

#define IMGUI_DEFINE_MATH_OPERATORS

#include "ImeUI.h"

#include "ImGuiThemeLoader.h"
#include "ImeWnd.hpp"
#include "common/WCharUtils.h"
#include "common/log.h"
#include "ime/ImeManagerComposer.h"
#include "imgui.h"
#include "tsf/LangProfileUtil.h"
#include "ui/ImeUIWidgets.h"
#include "utils/FocusGFxCharacterInfo.h"

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{

static constexpr ImVec4 RED_COLOR = {1.0F, 0.0F, 0.0F, 1.0F};

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

void ImeUI::ApplyUiSettings(const SettingsConfig &settingsConfig)
{
    m_fontSizeScale                = settingsConfig.fontSizeScale.Value();
    ImGui::GetIO().FontGlobalScale = m_fontSizeScale;

    m_fShowSettings = settingsConfig.showSettings.Value();
    { // Apply language config
        const auto lang = settingsConfig.language.Value();
        m_imeUIWidgets.SetUInt32Var("$Languages", 0);
        if (const auto langIt = std::ranges::find(m_translateLanguages, lang); langIt != m_translateLanguages.end())
        {
            m_translation.UseLanguage(lang.c_str());
            size_t index = std::distance(m_translateLanguages.begin(), langIt);
            index        = std::min(index, m_translateLanguages.size() - 1);
            m_imeUIWidgets.SetUInt32Var("$Languages", index);
        }
    }

    { // Apply theme config
        m_imeUIWidgets.SetUInt32Var("$Themes", 0);
        const auto &defaultTheme = settingsConfig.theme.Value();
        const auto  findIt       = std::ranges::find(m_themeNames, defaultTheme);
        if (findIt == m_themeNames.end())
        {
            ErrorNotifier::GetInstance().addError(
                std::format("Can't find theme {}, fallback to ImGui default theme.", defaultTheme)
            );
            ImGui::StyleColorsDark();
            return;
        }
        m_imeUIWidgets.SetUInt32Var("$Themes", std::distance(m_themeNames.begin(), findIt));
        auto &style = ImGui::GetStyle();
        m_uiThemeLoader.LoadTheme(defaultTheme, style);
    }
}

void ImeUI::SyncUiSettings(SettingsConfig &settingsConfig)
{
    settingsConfig.fontSizeScale.SetValue(m_fontSizeScale);
    settingsConfig.showSettings.SetValue(m_fShowSettings);

    if (const auto opt = m_imeUIWidgets.GetUInt32Var("$Languages"); opt)
    {
        settingsConfig.language.SetValue(m_translateLanguages[opt.value()]);
    }
    if (const auto opt = m_imeUIWidgets.GetUInt32Var("$Themes"); opt)
    {
        settingsConfig.theme.SetValue(m_themeNames[opt.value()]);
    }
}

void ImeUI::SetTheme()
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

void ImeUI::Draw()
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
        if (ImVec2 windowPos; UpdateImeWindowPos(windowPos))
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

auto ImeUI::UpdateImeWindowPos(ImVec2 &windowPos) const -> bool
{
    switch (ImeManagerComposer::GetInstance()->GetImeWindowPosUpdatePolicy())
    {
        case ImeManagerComposer::ImeWindowPosUpdatePolicy::BASED_ON_CURSOR:
            if (const auto *cursor = RE::MenuCursor::GetSingleton(); cursor != nullptr)
            {
                windowPos.x = cursor->cursorPosX;
                windowPos.y = cursor->cursorPosY;
            }
            return true;
        case ImeManagerComposer::ImeWindowPosUpdatePolicy::BASED_ON_CARET: {
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

void ImeUI::ShowToolWindow()
{
    m_toolWindowFlags &= ~ImGuiWindowFlags_NoInputs;
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

void ImeUI::RenderToolWindow()
{
    if (!m_fShowToolWindow)
    {
        return;
    }

    m_translation.UseSection("Tool Window");
    ImGui::Begin(TOOL_WINDOW_NAME.data(), &m_fShowToolWindow, m_toolWindowFlags);

    DrawSettings();

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

    if (ImGui::Button("\xf0\x9f\x93\x8c"))
    {
        m_toolWindowFlags |= ImGuiWindowFlags_NoInputs;
        m_fPinToolWindow               = true;
        ImGui::GetIO().MouseDrawCursor = false;
    }

    ImGui::SameLine();
    m_imeUIWidgets.Checkbox("$Settings", m_fShowSettings);

    ImGui::SameLine();
    ImeUIWidgets::RenderInputMethodChooseWidget(m_langProfileUtil, m_pImeWnd);

    ImGui::SameLine();
    if (State::GetInstance().Has(State::IN_ALPHANUMERIC))
    {
        ImGui::Text("ENG");
        ImGui::SameLine();
    }
    ImGui::End();
}

void ImeUI::DrawSettings()
{
    if (!m_fShowSettings)
    {
        return;
    }
    auto      *imeManager = ImeManagerComposer::GetInstance();
    const auto windowName = std::format("{}###SettingsWindow", m_translation.Get("$Settings"));
    if (ImGui::Begin(windowName.c_str(), &m_fShowSettings, ImGuiWindowFlags_NoNav))
    {
        m_translation.UseSection("Settings");
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10, 4));
        DrawSettingsContent(imeManager);
        ImGui::PopStyleVar();

        m_imeUIWidgets.Combo("$Themes", m_themeNames, [this](const std::string &name) {
            return m_uiThemeLoader.LoadTheme(name, ImGui::GetStyle());
        });
    }
    ImGui::End();
    imeManager->SyncImeStateIfDirty();
}

void ImeUI::DrawSettingsContent(ImeManagerComposer *imeManager)
{
    bool fEnableMod = ImeManager::IsModEnabled();
    m_imeUIWidgets.Checkbox("$Enable_Mod", fEnableMod, [](bool EnableMod) {
        return ImeManagerComposer::GetInstance()->NotifyEnableMod(EnableMod);
    });

    ImGui::SameLine();
    ImGui::Text("%s", m_translation.Get("$Font_Size_Scale"));
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::DragFloat(
            "##FontSizeScale",
            &m_fontSizeScale,
            0.05,
            SettingsConfig::MIN_FONT_SIZE_SCALE,
            SettingsConfig::MAX_FONT_SIZE_SCALE,
            "%.3f",
            ImGuiSliderFlags_NoInput
        ))
    {
        ImGui::GetIO().FontGlobalScale = m_fontSizeScale;
    }

    m_imeUIWidgets.Combo("$Languages", m_translateLanguages, [this](const std::string &lang) {
        return m_translation.UseLanguage(lang.c_str());
    });

    if (!fEnableMod)
    {
        return;
    }
    RenderSettingsState();

    m_imeUIWidgets.SeparatorText("$Features");

    RenderSettingsFocusManage();
    RenderSettingsImePosUpdatePolicy();
    bool fEnableUnicodePaste = imeManager->IsUnicodePasteEnabled();
    if (m_imeUIWidgets.Checkbox("$Enable_Unicode_Paste", fEnableUnicodePaste))
    {
        imeManager->SetEnableUnicodePaste(fEnableUnicodePaste);
    }

    ImGui::SameLine();
    bool fKeepImeOpen = imeManager->IsKeepImeOpen();
    if (m_imeUIWidgets.Checkbox("$Keep_Ime_Open", fKeepImeOpen))
    {
        imeManager->SetKeepImeOpen(fKeepImeOpen);
    }
}

void ImeUI::RenderSettingsState() const
{
    m_imeUIWidgets.SeparatorText("$States");
    m_imeUIWidgets.StateWidget("$Ime_Enabled", State::GetInstance().NotHas(State::IME_DISABLED));
    m_imeUIWidgets.StateWidget("$Ime_Focus", m_pImeWnd->IsFocused());
    ImGui::SameLine();
    m_imeUIWidgets.Button("$Force_Focus_Ime", [] {
        // ReSharper disable once CppExpressionWithoutSideEffects
        ImeManagerComposer::GetInstance()->ForceFocusIme();
    });
}

void ImeUI::RenderSettingsFocusManage()
{
    // Focus Manage widget
    auto *imeManager = ImeManagerComposer::GetInstance();

    m_imeUIWidgets.SeparatorText("$Focus_Manage");

    auto focusType = imeManager->GetFocusManageType();
    bool pressed   = m_imeUIWidgets.RadioButton("$Focus_Manage_Permanent", &focusType, FocusType::Permanent);
    ImGui::SameLine();
    pressed |= m_imeUIWidgets.RadioButton("$Focus_Manage_Temporary", &focusType, FocusType::Temporary);
    if (pressed)
    {
        imeManager->PopAndPushType(focusType);
    }
}

void ImeUI::RenderSettingsImePosUpdatePolicy()
{
    using Policy = ImeManagerComposer::ImeWindowPosUpdatePolicy;
    auto policy  = ImeManagerComposer::GetInstance()->GetImeWindowPosUpdatePolicy();
    m_translation.UseSection("Ime Window Pos");
    {
        m_imeUIWidgets.SeparatorText("$Policy");
        m_imeUIWidgets.RadioButton("$Update_By_Cursor", &policy, Policy::BASED_ON_CURSOR);
        ImGui::SameLine();
        m_imeUIWidgets.RadioButton("$Update_By_Caret", &policy, Policy::BASED_ON_CARET);
        ImGui::SameLine();
        m_imeUIWidgets.RadioButton("$Update_By_None", &policy, Policy::NONE);
        ImeManagerComposer::GetInstance()->SetImeWindowPosUpdatePolicy(policy);
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

} // namespace  SimpleIME
} // namespace LIBC_NAMESPACE_DECL