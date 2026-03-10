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
class SettingsWindow
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
