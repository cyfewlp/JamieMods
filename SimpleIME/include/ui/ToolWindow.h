//
// Created by jamie on 2026/3/10.
//

#pragma once

#include "Settings.h"
#include "fonts/FontBuilder.h"
#include "fonts/FontBuilderPanel.h"
#include "panels/AppearancePanel.h"

namespace Ime::UI
{
/**
 * TODO: may show a state info in AppBar or other place? [low priority]
 * show current input method?
 * @note Initialize FontManager will be time-consuming operation, it will load all installed system font by DWrite API during constructing.
 */
class ToolWindow
{
public:
    enum class Menu : int8_t
    {
        Appearance,
        FontBuilder,
        Behaviour,
    };

private:
    FontBuilder      m_fontBuilder;
    FontBuilderPanel m_fontBuilderView{};
    AppearancePanel  m_panelAppearance{};
    Menu             m_currentMenu = Menu::Appearance;

public:
    ToolWindow();
    ~ToolWindow();

    ToolWindow(const ToolWindow &)            = delete;
    ToolWindow &operator=(const ToolWindow &) = delete;
    ToolWindow(ToolWindow &&)                 = delete;
    ToolWindow &operator=(ToolWindow &&)      = delete;

    void Draw(Settings &settings);

private:
    void        DrawMenuAppearance(Settings &settings);
    void        DrawMenuFontBuilder(Settings &settings);
    void        DrawMenuBehaviour(Settings &settings) const;
    static void DrawFeatures(Settings &settings);
    static void DrawWindowPosUpdatePolicy(Settings &settings);
    void        DrawStates() const;
};
} // namespace Ime::UI
