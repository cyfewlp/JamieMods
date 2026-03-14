//
// Created by jamie on 2026/3/10.
//

#include "ui/ToolWindow.h"

#include "i18n/translator_manager.h"
#include "menu/MenuNames.h"
#include "utils/Utils.h"

namespace Ime::UI
{

namespace
{
void TogglePinned(bool &pinned, bool &showing)
{
    if (pinned)
    {
        pinned = false;
    }
    else if (showing)
    {
        showing = false;
    }
    else
    {
        showing = true;
    }
}
} // namespace

ToolWindow::ToolWindow(ImGuiKeyChord shortcut, std::string_view language) : m_shortcut(shortcut)
{
    i18n::SetTranslator(&m_translator);
    i18n::UpdateTranslator(language, "english");
}

ToolWindow::~ToolWindow()
{
    i18n::SetTranslator(nullptr);
}

auto ToolWindow::Draw(const LangProfile &activeLangProfile, const std::vector<LangProfile> &langProfiles, Settings &settings) -> bool
{
    bool showing = m_showing;
    if (ImGui::IsKeyChordPressed(m_shortcut))
    {
        TogglePinned(m_pinned, showing);
    }
    if (showing != m_showing)
    {
        Skyrim::ToggleMenu(ToolWindowMenuName, showing);
    }

    m_showing = showing;
    if (m_showing)
    {
        if (LanguageBar::Draw(m_pinned, activeLangProfile, langProfiles))
        {
            settings.appearance.showSettings = true;
        }

        if (settings.appearance.showSettings)
        {
            if (m_settingsWindow == nullptr)
            {
                m_settingsWindow = std::make_unique<UI::SettingsWindow>();
            }
            m_settingsWindow->Draw(settings);
        }
        else if (m_settingsWindow != nullptr)
        {
            m_settingsWindow.reset();
        }
    }
    return m_showing;
}
} // namespace Ime::UI
