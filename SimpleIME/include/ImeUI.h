#pragma once

#include "core/State.h"
#include "i18n/TranslatorHolder.h"
#include "ui/Settings.h"
#include "ui/fonts/FontBuilder.h"
#include "ui/fonts/FontBuilderPanel.h"
#include "ui/panels/AppearancePanel.h"

#include <vector>

namespace Ime
{
class ImeWnd;

constexpr auto MAX_ERROR_MESSAGE_COUNT = 10;

class ImeUI
{
    using State = Core::State;

public:
    explicit ImeUI(ImeWnd *pImeWnd) : m_pImeWnd(pImeWnd) {}

    ~ImeUI()                                 = default;
    ImeUI(const ImeUI &other)                = delete;
    ImeUI(ImeUI &&other) noexcept            = delete;
    ImeUI &operator=(const ImeUI &other)     = delete;
    ImeUI &operator=(ImeUI &&other) noexcept = delete;

    void Initialize();
    void ApplySettings(Settings::Appearance &appearance, ImGuiEx::M3::M3Styles &m3Styles);
    void ShowToolWindow();
    void DrawSettings(Settings &settings, ImGuiEx::M3::M3Styles &m3Styles);

    bool IsShowingToolWindow() const
    {
        return m_fShowToolWindow;
    }

    bool IsPinedToolWindow() const
    {
        return m_fPinToolWindow;
    }

private:
    void        DrawMenuAppearance(Settings &settings, ImGuiEx::M3::M3Styles &m3Styles);
    void        DrawMenuFontBuilder(Settings &settings, const ImGuiEx::M3::M3Styles &m3Styles);
    void        DrawMenuBehaviour(Settings &settings) const;
    static void DrawFeatures(Settings &settings);
    void        DrawStates() const;
    static void DrawWindowPosUpdatePolicy(Settings &settings);

    static constexpr auto TOOL_WINDOW_NAME = std::span("ToolWindow##SimpleIME");

    ImeWnd                  *m_pImeWnd = nullptr;
    std::vector<std::string> m_translateLanguages;
    FontBuilder              m_fontBuilder;
    FontBuilderPanel         m_fontBuilderView{};
    AppearancePanel          m_panelAppearance{};

    std::optional<TranslatorHolder::UpdateHandle> m_i18nHandle;

    bool m_fShowToolWindow = false;
    bool m_fPinToolWindow  = false;
};
} // namespace Ime
