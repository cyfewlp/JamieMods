//
// Created by jamie on 2026/3/10.
//

#include "ui/ToolWindow.h"

#include "i18n/translator_manager.h"
#include "menu/MenuNames.h"

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
    if (auto *const messageQueue = RE::UIMessageQueue::GetSingleton(); messageQueue != nullptr)
    {
        messageQueue->AddMessage(ToolWindowMenuName, RE::UI_MESSAGE_TYPE::kShow, nullptr);
    }
    i18n::UpdateTranslator(language, "english");
}

ToolWindow::~ToolWindow()
{
    if (auto *const messageQueue = RE::UIMessageQueue::GetSingleton(); messageQueue != nullptr)
    {
        messageQueue->AddMessage(ToolWindowMenuName, RE::UI_MESSAGE_TYPE::kHide, nullptr);
    }
    i18n::ReleaseTranslator();
}

auto ToolWindow::Draw(const LangProfile &activeLangProfile, const std::vector<LangProfile> &langProfiles, Settings &settings) -> bool
{
    if (ImGui::Shortcut(m_shortcut, ImGuiInputFlags_RouteGlobal))
    {
        TogglePinned(m_pinned, m_showing);
    }
    if (!m_showing)
    {
        return m_showing;
    }

    if (m_languageBar.Draw(m_pinned, activeLangProfile, langProfiles))
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
    else
    {
        m_settingsWindow.reset();
    }
    return m_showing;
}
} // namespace Ime::UI
