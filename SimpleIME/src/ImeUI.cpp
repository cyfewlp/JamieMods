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

#include <clocale>
#include <tchar.h>

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{

static constexpr ImVec4 RED_COLOR = {1.0F, 0.0F, 0.0F, 1.0F};

ImeUI::ImeUI(AppUiConfig const &uiConfig, ImeWnd *pImeWnd, ITextService *pTextService) : m_uiConfig(uiConfig)
{
    _tsetlocale(LC_ALL, _T(""));
    m_pTextService = pTextService;
    m_pImeWnd      = pImeWnd;
}

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
        auto      &defaultTheme = settingsConfig.theme.Value();
        const auto findIt       = std::ranges::find(m_themeNames, defaultTheme);
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

void ImeUI::RenderIme() const
{
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize;
    windowFlags |= ImGuiWindowFlags_NoDecoration;
    windowFlags |= ImGuiWindowFlags_AlwaysAutoResize;
    static bool g_fShowImeWindow = false, g_fImeWindowPosUpdated = false;
    auto       &state = State::GetInstance();
    if (state.Has(State::IME_DISABLED) || state.NotHas(State::IN_CAND_CHOOSING, State::IN_COMPOSING))
    {
        g_fShowImeWindow = false;
        return;
    }
    UpdateImeWindowPos(g_fShowImeWindow, g_fImeWindowPosUpdated);
    g_fShowImeWindow = true;

    ImGui::Begin(SKSE::PluginDeclaration::GetSingleton()->GetName().data(), nullptr, windowFlags);

    ImGui::SameLine();
    ImGui::BeginGroup();
    RenderCompWindow();

    ImGui::Separator();
    // render ime status window: language,
    if (state.Has(State::IN_CAND_CHOOSING))
    {
        RenderCandidateWindows();
    }
    ImGui::EndGroup();
    ImGui::End();
}

auto ImeUI::UpdateImeWindowPos(bool showIme, bool &updated) -> void
{
    if (ImeManagerComposer::GetInstance()->IsSupportOtherMod())
    {
        return;
    }
    switch (ImeManagerComposer::GetInstance()->GetImeWindowPosUpdatePolicy())
    {
        case ImeManagerComposer::ImeWindowPosUpdatePolicy::BASED_ON_CURSOR:
            if (auto *cursor = RE::MenuCursor::GetSingleton(); cursor != nullptr)
            {
                ImGui::SetNextWindowPos({cursor->cursorPosX, cursor->cursorPosY}, ImGuiCond_Appearing);
            }
            break;
        case ImeManagerComposer::ImeWindowPosUpdatePolicy::BASED_ON_CARET: {
            if (!showIme || !updated)
            {
                FocusGFxCharacterInfo::GetInstance().UpdateCaretCharBoundaries();
                if (UpdateImeWindowPosByCaret())
                {
                    updated = true;
                }
            }
            break;
        }
        default:;
    }
}

auto ImeUI::UpdateImeWindowPosByCaret() -> bool
{
    if (!State::GetInstance().HasAny(State::IN_CAND_CHOOSING, State::IN_COMPOSING))
    {
        return false;
    }

    auto &instance      = FocusGFxCharacterInfo::GetInstance();
    auto  imeWindowSize = ImGui::GetContentRegionAvail();
    if (imeWindowSize.x == 0 || imeWindowSize.y == 0)
    {
        return false;
    }
    const auto &charBoundaries = instance.CharBoundaries();
    auto       *renderer       = RE::BSGraphics::Renderer::GetSingleton();
    auto        screenSize     = renderer->GetScreenSize();
    bool        doUpdate       = false;

    ImVec2 windowPos = imeWindowSize + ImGui::GetCursorScreenPos();
    if (charBoundaries.bottom + imeWindowSize.y <= screenSize.height)
    {
        windowPos.x = charBoundaries.right;
        windowPos.y = charBoundaries.bottom;
        doUpdate    = true;
    }
    else if (charBoundaries.top - imeWindowSize.y >= 0)
    {
        windowPos.x = charBoundaries.right;
        windowPos.y = charBoundaries.top - imeWindowSize.y;
        doUpdate    = true;
    }

    if (doUpdate)
    {
        if (windowPos.x + imeWindowSize.x > screenSize.width)
        {
            windowPos.x = screenSize.width - imeWindowSize.x;
        }
        ImGui::SetNextWindowPos(windowPos, ImGuiCond_Appearing);
    }
    return true;
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

    RenderSettings();

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

void ImeUI::RenderSettings()
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
        do
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
                break;
            }
            RenderSettingsState();

            m_imeUIWidgets.SeparatorText("$Features");

            RenderSettingsFocusManage();
            if (!imeManager->IsSupportOtherMod())
            {
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
        } while (false);
        ImGui::PopStyleVar();

        m_imeUIWidgets.Combo("$Themes", m_themeNames, [this](const std::string &name) {
            return m_uiThemeLoader.LoadTheme(name, ImGui::GetStyle());
        });
    }
    ImGui::End();
    imeManager->SyncImeStateIfDirty();
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

void ImeUI::RenderCandidateWindows() const
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