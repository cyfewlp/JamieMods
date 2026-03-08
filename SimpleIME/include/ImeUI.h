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
    explicit ImeUI() = default;

    ~ImeUI()                                 = default;
    ImeUI(const ImeUI &other)                = delete;
    ImeUI(ImeUI &&other) noexcept            = delete;
    ImeUI &operator=(const ImeUI &other)     = delete;
    ImeUI &operator=(ImeUI &&other) noexcept = delete;

    void Initialize();
    void ApplySettings(Settings::Appearance &appearance);
    void DrawSettings(Settings &settings);

private:
    void        DrawMenuAppearance(Settings &settings);
    void        DrawMenuFontBuilder(Settings &settings);
    void        DrawMenuBehaviour(Settings &settings) const;
    static void DrawFeatures(Settings &settings);
    void        DrawStates() const;
    static void DrawWindowPosUpdatePolicy(Settings &settings);

    static constexpr auto TOOL_WINDOW_NAME = std::span("ToolWindow##SimpleIME");

    std::vector<std::string> m_translateLanguages;
    FontBuilder              m_fontBuilder;
    UI::FontBuilderPanel     m_fontBuilderView{};
    AppearancePanel          m_panelAppearance{};

    std::optional<TranslatorHolder::UpdateHandle> m_i18nHandle;
};
} // namespace Ime
