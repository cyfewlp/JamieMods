#pragma once

#include "common/i18n/Translator.h"
#include "common/imgui/Material3.h"
#include "core/State.h"
#include "i18n/TranslatorHolder.h"
#include "imgui.h"
#include "tsf/LangProfileUtil.h"
#include "ui/Settings.h"
#include "ui/fonts/FontBuilder.h"
#include "ui/fonts/FontBuilderPanel.h"
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
    explicit ImeUI(ImeWnd *pImeWnd, ImGuiEx::M3::M3Styles &styles)
        : m_pImeWnd(pImeWnd), m_styles(styles), m_fontBuilderView(styles), m_panelAppearance(styles)
    {
    }

    ~ImeUI();
    ImeUI(const ImeUI &other)                = delete;
    ImeUI(ImeUI &&other) noexcept            = delete;
    ImeUI &operator=(const ImeUI &other)     = delete;
    ImeUI &operator=(ImeUI &&other) noexcept = delete;

    bool Initialize(LangProfileUtil *pLangProfileUtil);
    void ApplyAppearanceSettings(Settings &settings);
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
    void        DrawInputMethodsCombo() const;
    void        DrawSettings(Settings &settings);
    void        DrawMenuAppearance(Settings &settings, bool appearing);
    void        DrawMenuFontBuilder(Settings &settings);
    void        DrawMenuBehaviour(Settings &settings);
    void        DrawModConfig(Settings &settings);
    void        DrawFontConfig(Settings &settings);
    static void DrawFeatures(Settings &settings);
    void        DrawSettingsContent(Settings &settings);
    void        DrawStates() const;
    static void DrawWindowPosUpdatePolicy(Settings &settings);
    void        LoadTranslation(std::string_view language) const;

    static void FillCommonStyleFields(ImGuiStyle &style, const Settings &settings);

    static bool DrawCombo(const char *label, const std::vector<std::string> &values, std::string &selected);

    static constexpr auto TOOL_WINDOW_NAME = std::span("ToolWindow##SimpleIME");

    ImeWnd                  *m_pImeWnd         = nullptr;
    LangProfileUtil         *m_langProfileUtil = nullptr;
    std::vector<std::string> m_translateLanguages;
    FontBuilder              m_fontBuilder;
    ImGuiEx::M3::M3Styles   &m_styles;
    FontBuilderPanel         m_fontBuilderView;
    AppearancePanel          m_panelAppearance;

    std::optional<TranslatorHolder::UpdateHandle> m_i18nHandle;

    bool m_fShowToolWindow = false;
    bool m_fPinToolWindow  = false;
};
} // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL
