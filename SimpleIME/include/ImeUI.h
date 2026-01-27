#pragma once

#include "common/imgui/Material3.h"
#include "common/imgui/ThemesLoader.h"
#include "common/utils.h"
#include "core/State.h"
#include "core/Translation.h"
#include "ime/ITextService.h"
#include "imgui.h"
#include "tsf/LangProfileUtil.h"
#include "ui/Settings.h"
#include "ui/fonts/FontBuilder.h"
#include "ui/fonts/FontBuilderView.h"
#include "ui/panels/AppearancePanel.h"

#include <vector>

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
class ImeWnd;

constexpr auto MAX_ERROR_MESSAGE_COUNT = 10;

class ImeUI
{
    using State = Core::State;

public:
    explicit ImeUI(ImeWnd *pImeWnd, ITextService *pTextService, ImGuiEx::M3::M3Styles &styles)
        : m_pImeWnd(pImeWnd), m_pTextService(pTextService),
          m_themesLoader(CommonUtils::GetInterfaceFile(ImGuiUtil::ThemesLoader::DEFAULT_THEME_FILE)), m_styles(styles),
          m_fontBuilderView(styles), m_panelAppearance(styles, m_translation)
    {
    }

    ~ImeUI();
    ImeUI(const ImeUI &other)                = delete;
    ImeUI(ImeUI &&other) noexcept            = delete;
    ImeUI &operator=(const ImeUI &other)     = delete;
    ImeUI &operator=(ImeUI &&other) noexcept = delete;

    bool Initialize(LangProfileUtil *pLangProfileUtil, const Settings &settings);
    void ApplyAppearanceSettings(Settings &settings);
    void ApplyTheme(Settings &settings);
    void Draw(const Settings &settings);
    void DrawToolWindow(Settings &settings);
    void ShowToolWindow();

    bool IsShowingToolWindow() const
    {
        return m_fShowToolWindow;
    }

    bool IsPinedToolWindow() const
    {
        return m_fPinToolWindow;
    }

private:
    void DrawInputMethodsCombo() const;
    void DrawSettings(Settings &settings);
    void DrawMenuAppearance(Settings &settings, bool appearing);
    void DrawMenuFontBuilder(Settings &settings);
    void DrawMenuBehaviour(Settings &settings);
    void DrawModConfig(Settings &settings);
    void DrawFontConfig(Settings &settings);
    void DrawFeatures(Settings &settings);
    void DrawSettingsContent(Settings &settings);
    void DrawStates() const;
    void DrawWindowPosUpdatePolicy(Settings &settings);
    void DrawCompWindow(const Settings &settings) const;
    void DrawCandidateWindows() const;
    auto Translate(const char *label) const -> const char *;

    static auto UpdateImeWindowPos(const Settings &settings, ImVec2 &windowPos) -> void;
    static auto UpdateImeWindowPosByCaret(ImVec2 &windowPos) -> void;
    auto        IsImeNeedRelayout() const -> bool;
    /**
     * @brief Ensure the current ImGui window is fully within the viewport.
     */
    static void ClampWindowToViewport(const ImVec2 &windowSize, ImVec2 &windowPos);

    static void FillCommonStyleFields(ImGuiStyle &style, const Settings &settings);

    static bool DrawCombo(const char *label, const std::vector<std::string> &values, std::string &selected);

    static constexpr auto TOOL_WINDOW_NAME = std::span("ToolWindow##SimpleIME");

    ImeWnd                  *m_pImeWnd         = nullptr;
    ITextService            *m_pTextService    = nullptr;
    LangProfileUtil         *m_langProfileUtil = nullptr;
    Translation              m_translation;
    std::vector<std::string> m_translateLanguages;
    ImVec2                   m_imeWindowSize = ImVec2(0, 0);
    ImVec2                   m_imeWindowPos  = ImVec2(0, 0);
    ImGuiUtil::ThemesLoader  m_themesLoader;
    FontBuilder              m_fontBuilder;
    ImGuiEx::M3::M3Styles   &m_styles;
    FontBuilderView          m_fontBuilderView;
    AppearancePanel          m_panelAppearance;

    bool m_fShowToolWindow = false;
    bool m_fPinToolWindow  = false;
};
} // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL
