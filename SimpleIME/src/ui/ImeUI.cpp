//
// Created by jamie on 2026/3/10.
//

#include "ui/ImeUI.h"

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

ImeUI::ImeUI(ImGuiKeyChord shortcut, std::string_view language) : m_shortcut(shortcut)
{
    i18n::SetTranslator(&m_translator);
    i18n::UpdateTranslator(language, "english");
}

ImeUI::~ImeUI()
{
    i18n::SetTranslator(nullptr);
}

auto ImeUI::Draw(const LangProfile &activeLangProfile, const std::vector<LangProfile> &langProfiles, Settings &settings) -> bool
{
    bool showing = m_showingLanguageBar;
    if (ImGui::IsKeyChordPressed(m_shortcut))
    {
        TogglePinned(m_pinnedLanguageBar, showing);
    }

    m_showingLanguageBar = showing;
    if (m_showingLanguageBar)
    {
        if (LanguageBar::Draw(m_pinnedLanguageBar, activeLangProfile, langProfiles))
        {
            settings.appearance.showSettings = true;
        }
    }
    if (m_showingLanguageBar && settings.appearance.showSettings)
    {
        if (m_toolWindow == nullptr)
        {
            m_toolWindow = std::make_unique<UI::ToolWindow>();
        }
        m_toolWindow->Draw(settings);
    }
    else if (m_toolWindow != nullptr)
    {
        m_toolWindow.reset();
    }
    return m_showingLanguageBar;
}
} // namespace Ime::UI
