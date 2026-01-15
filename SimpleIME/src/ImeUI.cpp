//
// Created by jamie on 25-1-21.
//

#define IMGUI_DEFINE_MATH_OPERATORS

#include "ImeUI.h"

#include "ImeWnd.hpp"
#include "common/WCharUtils.h"
#include "common/config.h"
#include "common/imgui/ErrorNotifier.h"
#include "common/imgui/ThemesLoader.h"
#include "common/log.h"
#include "common/utils.h"
#include "configs/CustomMessage.h"
#include "core/State.h"
#include "icons.h"
#include "ime/ImeController.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "menu/MenuNames.h"
#include "tsf/LangProfileUtil.h"
#include "ui/Settings.h"
#include "ui/TaskQueue.h"
#include "utils/FocusGFxCharacterInfo.h"
#include "utils/FontManager.h"

#include <RE/M/MenuCursor.h>
#include <RE/U/UIMessage.h>
#include <RE/U/UIMessageQueue.h>
#include <SKSE/Interfaces.h>
#include <WinUser.h>
#include <cassert>
#include <cfloat>
#include <cguid.h>
#include <climits>
#include <cstdint>
#include <expected>
#include <format>
#include <math.h>
#include <minwindef.h>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <winnt.h>

#pragma comment(lib, "dwrite.lib")

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
static std::string PREVIEW_TEXT = R"(!@#$%^&*()_+-=[]{}|;':",.<>?/
-- Unicode & Fallback --
Latín: áéíóú ñ  |  FullWidth: ＡＢＣ１２３
CJK: 繁體中文测试 / 简体中文测试 / 日本語 / 한국어
-- Emoji & Variation --
Icons: 🥰💀✌︎🌴🐢🐐🍄🍻👑📸😬👀🚨🏡
New: 🐦‍🔥 🍋‍🟩 🍄‍🟫 🙂‍↕️ 🙂‍↔️
-- Skyrim Immersion --
Dovah: Dovahkiin, naal ok zin los vahriin!
"I used to be an adventurer like you..."
)";

ImeUI::~ImeUI()
{
    if (m_pTextService != nullptr)
    {
        m_langProfileUtil->Release();
    }
}

bool ImeUI::Initialize(LangProfileUtil *pLangProfileUtil, const Settings &settings)
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
    if (!m_translation.GetTranslateLanguages(settings.resources.translationDir, m_translateLanguages))
    {
        log_warn("Failed load translation languages.");
    }
    m_fontBuilder.Initialize();
    return true;
}

void ImeUI::ApplyAppearanceSettings(Settings &settings)
{
    auto &appearance = settings.appearance;
    { // Apply language config
        if (const auto langIt = std::ranges::find(m_translateLanguages, appearance.language);
            langIt == m_translateLanguages.end())
        {
            appearance.language = "english";
        }
        m_translation.UseLanguage(appearance.language.c_str());
    }

    ApplyTheme(settings);
}

void ImeUI::ApplyTheme(Settings &settings)
{
    m_themesLoader.LoadThemes();

    auto       &appearance = settings.appearance;
    const auto &themes     = m_themesLoader.GetThemes();

    ImGuiStyle                       style;
    std::expected<void, std::string> expected = std::unexpected("Theme not found");
    if (appearance.themeIndex < themes.size())
    {
        if (themes[appearance.themeIndex].name == appearance.theme)
        {
            expected = m_themesLoader.UseTheme(appearance.themeIndex, style);
        }
    }
    else
    {
        const auto findIt     = std::lower_bound(themes.begin(), themes.end(), ImGuiUtil::Theme(0, appearance.theme));
        appearance.themeIndex = static_cast<size_t>(std::distance(themes.begin(), findIt));
        if (appearance.themeIndex < themes.size())
        {
            expected = m_themesLoader.UseTheme(appearance.themeIndex, style);
        }
    }
    if (expected)
    {
        FillCommonStyleFields(style, settings);
        ImGui::GetStyle() = style;
    }
    else
    {
        ErrorNotifier::GetInstance().Warning(
            std::format("Can't find theme {}, fallback to ImGui default theme: {}", appearance.theme, expected.error())
        );
        if (m_themesLoader.UseTheme(0, ImGui::GetStyle()))
        {
            appearance.theme      = "Dark";
            appearance.themeIndex = 0;
        }
    }
    ImGuiUtil::ThemesLoader::Cleanup();
    ImGui::GetStyle().FontSizeBase = appearance.fontSize;
}

void ImeUI::NewFrame()
{
    m_fontBuilder.BuildPreviewFont();
}

void ImeUI::FontBuilder::UpdatePreviewFont(const FontInfo &fontInfo)
{
    const auto filePath = FontManager::GetFontFilePath(fontInfo);
    if (!filePath.empty())
    {
        m_previewFont.SetProperty(fontInfo.name, filePath);
    }
}

void ImeUI::FontBuilder::BuildPreviewFont()
{
    if (!m_previewFont.IsWantUpdate())
    {
        return;
    }
    ReleasePreviewFont();
    ImFontConfig config;
    config.FontDataOwnedByAtlas = false;
    const auto &path            = m_previewFont.GetFilePath();
    m_previewFont.SetImFont(ImGui::GetIO().Fonts->AddFontFromFileTTF(path.c_str(), 0, &config));
}

void ImeUI::Draw(const Settings &settings)
{
    static bool shouldRelayout = true;
    static bool imeAppearing   = true;
    const auto &state          = State::GetInstance();
    const auto  id             = SKSE::PluginDeclaration::GetSingleton()->GetName();
    if (state.ImeDisabled() || !state.IsImeInputting() /* || !state.HasAny(State::KEYBOARD_OPEN, State::IME_OPEN)*/)
    {
        imeAppearing   = true;
        shouldRelayout = true;
        ImGui::CloseCurrentPopup();
        return;
    }
    if (imeAppearing)
    {
        imeAppearing = false;
        UpdateImeWindowPos(settings, m_imeWindowPos);
        ImGui::OpenPopup(id.data());
    }
    if (shouldRelayout)
    {
        shouldRelayout = false;
        ClampWindowToViewport(m_imeWindowSize, m_imeWindowPos);
        ImGui::SetNextWindowPos(m_imeWindowPos);
    }

    auto flags = ImGuiWindowFlags_NoFocusOnAppearing;
    if (ImGui::BeginPopup(id.data(), flags))
    {
        if (state.Has(State::IN_COMPOSING))
        {
            DrawCompWindow(settings);
        }

        ImGui::Separator();
        // render ime status window: language,
        if (state.Has(State::IN_CAND_CHOOSING))
        {
            DrawCandidateWindows();
        }
        m_imeWindowSize = ImGui::GetWindowSize();
        ImGui::EndPopup();
    }
    if (IsImeNeedRelayout())
    {
        shouldRelayout = true;
    }
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

void ImeUI::DrawToolWindow(Settings &settings)
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
    ImGui::SetItemTooltip("%s", Translate("$Settings"));

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

void ImeUI::DrawSettings(Settings &settings)
{
    if (!settings.appearance.showSettings)
    {
        return;
    }
    auto      *imeManager = ImeController::GetInstance();
    const auto windowName = std::format("{}###SettingsWindow", m_translation.Get("$Settings"));
    if (ImGui::Begin(windowName.c_str(), &settings.appearance.showSettings))
    {
        m_translation.UseSection("Settings");
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10, 4));
        if (ImGui::BeginTabBar("SettingsTabBar", ImGuiTabBarFlags_None))
        {
            DrawSettingsContent(settings);
            ImGui::EndTabBar();
        }
        ImGui::PopStyleVar();
        if (!settings.appearance.showSettings)
        {
            ImGuiUtil::ThemesLoader::Cleanup();
        }
    }
    ImGui::End();
    imeManager->SyncImeStateIfDirty();
}

void ImeUI::DrawModConfig(Settings &settings)
{
    bool enableMod = settings.enableMod;
    if (ImGui::Checkbox(Translate("$Enable_Mod"), &enableMod))
    {
        ImeController::GetInstance()->EnableMod(enableMod);
    }
    ImGui::SetItemTooltip("%s", Translate("$Enable_Mod_Tooltip"));

    if (ImGui::TreeNode(Translate("$Font")))
    {
        DrawFontConfig(settings);
        ImGui::TreePop();
    }

    if (DrawCombo(Translate("$Languages"), m_translateLanguages, settings.appearance.language))
    {
        m_translation.UseLanguage(settings.appearance.language.c_str());
    }

    if (ImGui::BeginCombo(Translate("$Themes"), settings.appearance.theme.c_str()))
    {
        size_t idx = 0;
        for (const auto &theme : m_themesLoader.GetThemes())
        {
            ImGui::PushID(static_cast<int>(idx));
            const bool isSelected = settings.appearance.themeIndex == idx;
            if (ImGui::Selectable(theme.name.c_str(), isSelected) && !isSelected)
            {
                ImGuiStyle style;
                if (m_themesLoader.UseTheme(idx, style))
                {
                    settings.appearance.themeIndex    = idx;
                    settings.appearance.theme         = theme.name;
                    settings.appearance.fontSizeScale = ImGui::GetStyle().FontScaleMain;
                    FillCommonStyleFields(style, settings);
                    ImGui::GetStyle() = style;
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
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0F);
        ImGui::TextUnformatted("Themes provided by ImThemes (https://github.com/Patitotective/ImThemes)");
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void ImeUI::DrawFontConfig(Settings &settings)
{
    ImGui::SliderFloat(Translate("$Font_Size"), &settings.fontSizeTemp, 10.0F, 100.0F);
    ImGui::SameLine();
    if (ImGui::Button(std::format("{} {}", ICON_OCT_CHECK, Translate("$Apply")).c_str()))
    {
        settings.appearance.fontSize = settings.fontSizeTemp;
    }

    ImGui::DragFloat(
        Translate("$Font_Size_Scale"),
        &ImGui::GetStyle().FontScaleMain,
        0.05,
        Settings::MIN_FONT_SIZE_SCALE,
        Settings::MAX_FONT_SIZE_SCALE,
        "%.3f",
        ImGuiSliderFlags_NoInput
    );
    m_fontBuilderView.Draw(m_fontBuilder, settings);
}

void ImeUI::DrawFeatures(Settings &settings)
{
    DrawWindowPosUpdatePolicy(settings);

    ImGui::Checkbox(Translate("$Enable_Unicode_Paste"), &settings.input.enableUnicodePaste);
    ImGui::SetItemTooltip("%s", Translate("$Enable_Unicode_Paste_Tooltip"));

    ImGui::SameLine();
    if (ImGui::Checkbox(Translate("$Keep_Ime_Open"), &settings.input.keepImeOpen))
    {
        ImeController::GetInstance()->MarkDirty();
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
    if (ImGui::BeginTabItem(Translate("$Features")))
    {
        DrawStates();
        DrawFeatures(settings);
        ImGui::EndTabItem();
    }
}

auto ImeUI::DrawCombo(const char *label, const std::vector<std::string> &values, std::string &selected) -> bool
{
    bool clicked = false;
    if (ImGui::BeginCombo(label, selected.c_str()))
    {
        int32_t idx = 0;
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

    constexpr auto STATE_ACTIVE_COLOR = ImVec4(0.35F, 0.75F, 1.0F, 1.0F);
    const auto    &state              = State::GetInstance();
    ImGui::AlignTextToFramePadding();
    ImGui::TextColored(
        state.NotHas(State::IME_DISABLED) ? STATE_ACTIVE_COLOR : inactiveColor, "[ %s ]", ICON_FA_KEYBOARD
    );
    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", Translate("$IME"));
    ImGui::SetItemTooltip("%s", Translate("$Ime_Enabled_Tooltip"));

    ImGui::SameLine();

    ImGui::AlignTextToFramePadding();
    ImGui::TextColored(m_pImeWnd->IsFocused() ? STATE_ACTIVE_COLOR : inactiveColor, "[ %s ]", ICON_FA_CROSSHAIR);
    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", Translate("$FOCUS"));
    ImGui::SetItemTooltip("%s", Translate("$Ime_Focus_Tooltip"));

    ImGui::SameLine(0, ImGui::GetFontSize());
    ImGui::TextDisabled("|");
    ImGui::SameLine(0, ImGui::GetFontSize());
    if (ImGui::Button(Translate("$Force_Focus_Ime")))
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
    m_translation.UseSection("Ime Window Pos");
    {
        ImGui::SeparatorText(Translate("$Policy"));

        RadioButton(Translate("$Update_By_Cursor"), &settings.input.posUpdatePolicy, Policy::BASED_ON_CURSOR);
        ImGui::SetItemTooltip("%s", Translate("$Update_By_Cursor_Tooltip"));

        ImGui::SameLine();
        RadioButton(Translate("$Update_By_Caret"), &settings.input.posUpdatePolicy, Policy::BASED_ON_CARET);
        ImGui::SetItemTooltip("%s", Translate("$Update_By_Caret_Tooltip"));

        ImGui::SameLine();
        RadioButton(Translate("$Update_By_None"), &settings.input.posUpdatePolicy, Policy::NONE);
        ImGui::SetItemTooltip("%s", Translate("$Update_By_None_Tooltip"));
    }
    m_translation.UseSection("Settings");
}

void ImeUI::DrawCompWindow(const Settings &settings) const
{
    static float CursorAnim = 0.F;

    CursorAnim += ImGui::GetIO().DeltaTime;

    const ImVec4 highLightText = ImGui::GetStyle().Colors[ImGuiCol_TextLink];
    const auto  &textEditor    = m_pTextService->GetTextEditor();

    const auto &editorText = textEditor.GetText();
    LONG        acpStart   = 0;
    LONG        acpEnd     = 0;
    textEditor.GetSelection(&acpStart, &acpEnd);

    bool success = true;

    const size_t textSize = editorText.size();
    acpStart              = std::max(0L, acpStart);
    acpEnd                = std::max(0L, acpEnd);

    acpStart = std::min(static_cast<long>(textSize), acpStart);
    acpEnd   = std::min(static_cast<long>(textSize), acpEnd);

    if (acpEnd < acpStart)
    {
        std::swap(acpStart, acpEnd);
    }

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
    if (fmodf(CursorAnim, 1.2F) <= 0.8F)
    {
        ImVec2 const cursorScreenPos = ImGui::GetCursorScreenPos();
        ImRect const cursorScreenRect(
            cursorScreenPos.x,
            cursorScreenPos.y - ImGui::GetFontSize() + 0.5f,
            cursorScreenPos.x + 1.0f,
            cursorScreenPos.y - 1.5f
        );
        ImGui::GetWindowDrawList()->AddLine(
            cursorScreenRect.Min,
            cursorScreenRect.GetBL(),
            ImGui::GetColorU32(ImGuiCol_InputTextCursor),
            1.0F * settings.dpiScale
        );
    }

    success = success && static_cast<size_t>(acpStart) < editorText.size();
    if (success)
    {
        std::string caretToEnd;
        auto        subFromCaret = editorText.substr(acpStart);
        success = WCharUtils::ToString(subFromCaret.c_str(), static_cast<int>(subFromCaret.size()), caretToEnd);
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

class ImGuiStyleVarScope
{
    int count = 0;

public:
    void Push(ImGuiStyleVar var, const ImVec2 &value)
    {
        ImGui::PushStyleVar(var, value);
        count++;
    }

    ~ImGuiStyleVarScope()
    {
        ImGui::PopStyleVar(count);
    }
};

struct ImGuiColorScope
{
    ImGuiColorScope(const ImGuiCol colorIndex, const ImU32 color)
    {
        ImGui::PushStyleColor(colorIndex, color);
    }

    ImGuiColorScope(const ImGuiCol colorIndex, const ImVec4 color)
    {
        ImGui::PushStyleColor(colorIndex, color);
    }

    ~ImGuiColorScope()
    {
        ImGui::PopStyleColor();
    }
};

void ImeUI::DrawCandidateWindows() const
{
    const auto  &candidateUi   = m_pTextService->GetCandidateUi();
    static DWORD hovered       = ULONG_MAX;
    const auto   candidateList = candidateUi.CandidateList();
    if (!candidateList.empty())
    {

        DWORD              index      = 0;
        DWORD              clicked    = candidateList.size();
        DWORD              anyHovered = candidateList.size();
        ImGuiStyleVarScope styleVarScope;
        styleVarScope.Push(ImGuiStyleVar_FramePadding, {5, 2});
        styleVarScope.Push(ImGuiStyleVar_ItemSpacing, {0, 0});

        auto           &style = ImGui::GetStyle();
        ImGuiColorScope buttonColorScope(ImGuiCol_Button, 0);
        for (const auto &candidate : candidateList)
        {
            ImGui::PushID(static_cast<int>(index));
            std::optional<ImGuiColorScope> buttonColorScope1;
            std::optional<ImGuiColorScope> textColorScope;
            if (hovered == index)
            {
                buttonColorScope1.emplace(ImGuiCol_Button, style.Colors[ImGuiCol_Button]);
            }
            if (index == candidateUi.Selection())
            {
                textColorScope.emplace(ImGuiCol_Text, style.Colors[ImGuiCol_TextLink]);
            }
            if (ImGui::Button(candidate.c_str()))
            {
                clicked = index;
            }
            buttonColorScope1.reset();
            textColorScope.reset();
            if (ImGui::IsItemHovered())
            {
                anyHovered = index;
            }
            ImGui::SameLine();
            ImGui::PopID();
            index++;
        }
        hovered = anyHovered;
        if (clicked < candidateList.size())
        {
            TaskQueue::GetInstance().AddImeThreadTask([this, clicked] {
                m_pTextService->CommitCandidate(m_pImeWnd->GetHWND(), clicked);
            });
            PostMessageA(m_pImeWnd->GetHWND(), CM_EXECUTE_TASK, 0, 0);
        }
    }
}

inline auto ImeUI::Translate(const char *label) const -> const char *
{
    return m_translation.Get(label);
}

auto ImeUI::UpdateImeWindowPos(const Settings &settings, ImVec2 &windowPos) -> void
{
    switch (settings.input.posUpdatePolicy)
    {
        case Settings::WindowPosUpdatePolicy::BASED_ON_CURSOR:
            if (const auto *cursor = RE::MenuCursor::GetSingleton(); cursor != nullptr)
            {
                windowPos.x = cursor->cursorPosX;
                windowPos.y = cursor->cursorPosY;
            }
            break;
        case Settings::WindowPosUpdatePolicy::BASED_ON_CARET: {
            FocusGFxCharacterInfo::GetInstance().UpdateCaretCharBoundaries();
            UpdateImeWindowPosByCaret(windowPos);
            break;
        }
        default:;
    }
}

auto ImeUI::UpdateImeWindowPosByCaret(ImVec2 &windowPos) -> void
{
    const auto &instance       = FocusGFxCharacterInfo::GetInstance();
    const auto &charBoundaries = instance.CharBoundaries();

    windowPos.x = charBoundaries.left;
    windowPos.y = charBoundaries.bottom;
}

auto ImeUI::IsImeNeedRelayout() const -> bool
{
    const auto &viewport = ImGui::GetMainViewport();

    return m_imeWindowPos.x + m_imeWindowSize.x > viewport->Size.x + viewport->Pos.x ||
           m_imeWindowPos.y + m_imeWindowSize.y > viewport->Size.y + viewport->Pos.y;
}

void ImeUI::ClampWindowToViewport(const ImVec2 &windowSize, ImVec2 &windowPos)
{
    const auto &viewport   = ImGui::GetMainViewport();
    const float viewRight  = viewport->Pos.x + viewport->Size.x;
    const float viewBottom = viewport->Pos.y + viewport->Size.y;
    windowPos.x            = std::max(windowPos.x, viewport->Pos.x);
    windowPos.y            = std::max(windowPos.y, viewport->Pos.y);

    if (windowSize.x < viewport->Size.x && windowPos.x + windowSize.x > viewRight)
    {
        windowPos.x = viewRight - windowSize.x;
    }
    if (windowSize.y < viewport->Size.y && windowPos.y + windowSize.y > viewBottom)
    {
        windowPos.y = viewBottom - windowSize.y;
    }

    const ImVec2 min            = windowPos;
    const ImVec2 max            = {windowPos.x + windowSize.x, windowPos.y + windowSize.y};
    const auto  &instance       = FocusGFxCharacterInfo::GetInstance();
    const auto  &charBoundaries = instance.CharBoundaries();
    if (charBoundaries.top < max.y && charBoundaries.bottom > min.y && charBoundaries.left < max.x &&
        charBoundaries.right > min.x) // is overlaps?
    {
        // Move the window above the boundary
        const float newY = charBoundaries.top - windowSize.y;
        if (newY >= viewport->Pos.y)
        {
            windowPos.y = newY;
        }
    }
}

void ImeUI::FillCommonStyleFields(ImGuiStyle &style, const Settings &settings)
{
    if (settings.dpiScale != 1.0F)
    {
        style.ScaleAllSizes(settings.dpiScale);
    }
    style.FontScaleDpi = settings.dpiScale;
}

auto ImeUI::FontBuilder::SetBaseFont() -> void
{
    if (m_baseFont != nullptr)
    {
        ImGui::GetIO().Fonts->RemoveFont(m_baseFont);
        m_fontNames.clear();
        m_fontPathList.clear();
    }
    m_baseFont = m_previewFont.GetImFont();

    // Transfer memory ownership to ImGui
    for (const auto &config : m_baseFont->Sources)
    {
        config->FontDataOwnedByAtlas = true;
    }

    m_fontNames.push_back(std::move(m_previewFont.GetFullName()));
    m_fontPathList.push_back(std::move(m_previewFont.GetFilePath()));
    m_previewFont.Reset();
}

void ImeUI::FontBuilder::MergeFont()
{
    if (m_previewFont.GetImFont() == nullptr)
    {
        return;
    }
    const auto *previewFontCfg = m_previewFont.GetImFont()->Sources[0];
    assert(!previewFontCfg->FontDataOwnedByAtlas && "Keep FontDataOwnedByAtlas to avoid font data copy!");

    ImFontConfig config;
    ImStrncpy(config.Name, previewFontCfg->Name, IM_COUNTOF(config.Name));
    config.OversampleH = 1;
    config.OversampleV = 1;
    config.PixelSnapH  = true;
    config.MergeMode   = true;
    config.DstFont     = m_baseFont;
    if (!previewFontCfg->FontDataOwnedByAtlas)
    {
        // Dont set FontDataOwnedByAtlas to false.
        config.FontData     = previewFontCfg->FontData;
        config.FontDataSize = previewFontCfg->FontDataSize;
    }
    else // should not reachable
    {
        config.FontData     = ImGui::MemAlloc(previewFontCfg->FontDataSize);
        config.FontDataSize = previewFontCfg->FontDataSize;
        std::memcpy(config.FontData, previewFontCfg->FontData, config.FontDataSize);
    }

    auto &io = ImGui::GetIO();
    io.Fonts->AddFont(&config);
    io.Fonts->RemoveFont(m_previewFont.GetImFont());

    m_fontNames.push_back(std::move(m_previewFont.GetFullName()));
    m_fontPathList.push_back(std::move(m_previewFont.GetFilePath()));
    m_previewFont.Reset();
}

void ImeUI::FontBuilder::Preview()
{
    if (auto *imFont = m_previewFont.GetImFont(); /**/
        m_previewFont.IsFontOwner() && imFont != nullptr)
    {
        ImGui::GetIO().Fonts->RemoveFont(imFont);
    }
    m_previewFont.SetPreviewImFont(m_baseFont);
}

void ImeUI::FontBuilder::SetAsDefault(Settings &settings)
{
    auto &io = ImGui::GetIO();
    if (io.FontDefault != m_baseFont)
    {
        io.Fonts->RemoveFont(io.FontDefault);
        io.FontDefault = m_baseFont;
    }

    ImFontConfig config;
    config.MergeMode   = true;
    config.OversampleH = 1;
    config.OversampleV = 1;
    config.PixelSnapH  = true;
    config.DstFont     = m_baseFont;

    // Note: Automatic detection of existing icon glyphs is skipped due
    // to implementation complexity.
    const auto iconFile = CommonUtils::GetInterfaceFile(Settings::ICON_FILE);
    io.Fonts->AddFontFromFileTTF(iconFile.c_str(), 0.0F, &config);

    settings.resources.fontPathList = std::move(m_fontPathList);

    m_baseFont = nullptr;
    m_fontNames.clear();
}

void ImeUI::FontBuilder::Reset()
{
    if (m_baseFont != nullptr)
    {
        ImGui::GetIO().Fonts->RemoveFont(m_baseFont);
    }
    m_baseFont = nullptr;
    m_fontNames.clear();
    m_fontPathList.clear();
}

void ImeUI::FontBuilder::ReleasePreviewFont()
{
    if (m_previewFont.IsFontOwner() && m_previewFont.GetImFont() != nullptr)
    {
        for (const auto &config : m_previewFont.GetImFont()->Sources)
        {
            if (!config->FontDataOwnedByAtlas)
            {
                IM_FREE(config->FontData);
            }
        }
        ImGui::GetIO().Fonts->RemoveFont(m_previewFont.GetImFont());
    }
    m_previewFont.SetPreviewImFont(nullptr);
}

void ImeUI::FontBuilderView::Draw(FontBuilder &fontBuilder, Settings &settings)
{
    ImGui::SeparatorText(m_translation["$Font_Builder"]);

    // draw chosen font information
    if (fontBuilder.IsBuilding() &&
        ImGui::BeginTable(
            "BasicFontInfo",
            2,
            ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit,
            ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 3)
        ))
    {
        int idx = 0;
        for (const auto &name : fontBuilder.GetFontNames())
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%d", idx + 1);
            ImGui::TableNextColumn();
            ImGui::Text("%s", name.c_str());
            idx++;
        }
        ImGui::EndTable();
        ImGui::BeginDisabled(!fontBuilder.IsBuilding());
        if (ImGui::Button(m_translation["$Font_Builder_SetAsDefault"]))
        {
            fontBuilder.SetAsDefault(settings);
        }

        ImGui::SameLine();
        if (ImGui::Button(m_translation["$Font_Builder_Reset"]))
        {
            fontBuilder.Reset();
        }

        ImGui::SameLine();
        if (ImGui::Button(m_translation["$Font_Builder_Preview"]))
        {
            fontBuilder.Preview();
        }
        ImGui::EndDisabled();

        ImGui::Separator();
    }

    if (ImGui::Button(ICON_MD_HELP_BOX))
    {
        ImGui::OpenPopup(TITLE_HELP);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_WARNING))
    {
        ImGui::OpenPopup(TITLE_WARNINGS);
    }
    DrawHelpModal();
    DrawWarningsModal();

    if (DrawFontViewer(fontBuilder, settings))
    {
        if (!fontBuilder.IsBuilding())
        {
            fontBuilder.SetBaseFont();
        }
        else // merge to base font
        {
            fontBuilder.MergeFont();
        }
    }
}

bool ImeUI::FontBuilderView::DrawFontViewer(FontBuilder &fontBuilder, const Settings &settings)
{
    static int selectedIndex = -1;

    constexpr auto MAX_ROWS         = 12;
    const float    TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();
    const ImVec2   FontViewerSize   = {200.0F, TEXT_BASE_HEIGHT * MAX_ROWS};

    bool        applied     = false;
    const auto &previewFont = fontBuilder.GetPreviewFont();

    ImGui::PushID(1);
    ImGui::BeginDisabled(!previewFont.IsCommittable());
    if (ImGui::Button(std::format("{} {}", ICON_MD_CONTENT_SAVE_MOVE, m_translation["$Add"]).c_str()))
    {
        if (selectedIndex >= 0)
        {
            applied       = true;
            selectedIndex = -1;
        }
    }
    ImGui::SameLine();
    ImGui::TextWrapped("%s %s", ICON_FA_FILE, previewFont.GetFilePath().c_str());
    ImGui::EndDisabled();
    ImGui::PopID();

    ImGui::BeginChild("#FontViewer", FontViewerSize, ImGuiChildFlags_ResizeX | ImGuiChildFlags_Borders);
    constexpr auto flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInner | ImGuiTableFlags_RowBg |
                           ImGuiTableFlags_NoHostExtendX;

    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F, ImGuiInputFlags_Tooltip);
    ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
    if (ImGui::InputTextWithHint(
            "##Filter",
            "search font by name",
            m_filter.InputBuf,
            IM_COUNTOF(m_filter.InputBuf),
            ImGuiInputTextFlags_EscapeClearsAll
        ))
    {
        m_filter.Build();
    }
    ImGui::PopItemFlag();

    if (ImGui::BeginTable("#InstalledFonts", 2, flags, ImVec2(0.0F, TEXT_BASE_HEIGHT * MAX_ROWS)))
    {
        int idx = 0;
        for (const auto &fontInfo : fontBuilder.GetFontManager().GetFontInfoList())
        {
            if (!m_filter.PassFilter(fontInfo.name.c_str()))
            {
                continue;
            }
            ImGui::TableNextRow();
            ImGui::PushID(idx);

            ImGui::TableNextColumn();
            ImGui::Text("%d", idx + 1);

            ImGui::TableNextColumn();
            const bool selected = selectedIndex == idx;
            if (ImGui::Selectable(fontInfo.name.c_str(), selected) && !selected)
            {
                selectedIndex = idx;
                fontBuilder.UpdatePreviewFont(fontInfo);
            }
            if (selected)
            {
                ImGui::SetItemDefaultFocus();
            }
            ImGui::PopID();

            idx++;
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("#FontPreviewer");
    {
        if (previewFont.GetImFont() != nullptr)
        {
            ImGui::PushFont(previewFont.GetImFont(), settings.fontSizeTemp);

            ImGui::InputTextMultiline(
                "##PreviewText",
                PREVIEW_TEXT.data(),
                PREVIEW_TEXT.capacity() + 1,
                ImVec2(-FLT_MIN, TEXT_BASE_HEIGHT * MAX_ROWS),
                ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_WordWrap
            );
            ImGui::PopFont();
        }
    }
    ImGui::EndChild();

    return applied;
}

void ImeUI::FontBuilderView::DrawHelpModal() const
{
    if (ImGui::BeginPopupModal(TITLE_HELP, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("%s", m_translation["$Font_Builder_Help1"]);
        ImGui::Text("%s", m_translation["$Font_Builder_Help2"]);
        ImGui::NewLine();
        ImGui::Text("%s", m_translation["$Font_Builder_Help3"]);

        ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x * 0.5F - ImGui::GetFontSize());
        if (ImGui::Button(ICON_FA_OK_SIGN))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void ImeUI::FontBuilderView::DrawWarningsModal() const
{
    if (ImGui::BeginPopupModal(TITLE_WARNINGS, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("%s %s", ICON_FA_WARNING, m_translation["$Font_Builder_Warning1"]);
        ImGui::NewLine();
        ImGui::Text("%s %s", ICON_FA_WARNING, m_translation["$Font_Builder_Warning2"]);

        ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x * 0.5F - ImGui::GetFontSize());
        if (ImGui::Button(ICON_FA_OK_SIGN))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

} // namespace  SimpleIME
} // namespace LIBC_NAMESPACE_DECL